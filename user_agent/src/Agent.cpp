
/*! \file   Agent.cpp

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Auction Manager System (NETAUM).

    NETAUM is free software; you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    NETAUM is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    
    Code based on Netmate

    $Id: Agent.cpp 748 2015-08-24 14:50:00 amarentes $
*/

#include "ParserFcts.h"
#include "logfile.h"
#include "httpd.h"
#include "Agent.h"
#include "EventAgent.h"
#include "ConstantsAgent.h"
#include "anslp_ipap_message.h"
#include "anslp_ipap_xml_message.h"

using namespace auction;


logfile commonlog("Agent.log", anslp::anslp_config::USE_COLOURS);
logfile &protlib::log::DefaultLog(commonlog);

// globals in AuctionManager class
int Agent::s_sigpipe[2];
int Agent::enableCtrl = 0;

// global timeout flag
int g_timeout = 0;

// remove newline from end of C string
static inline char *noNewline( char* s )
{
    char *p = strchr( s, '\n' );
    if (p != NULL) {
        *p = '\0';
    }
    return s;
}


/* ------------------------- Agent ------------------------- */

Agent::Agent( int argc, char *argv[])
    :  pprocThread(0)
{

    // record start time for later output
  startTime = ::time(NULL);

    try {

        if (pipe((int *)&s_sigpipe) < 0) {
            throw Error("failed to create signal pipe");
        }
				
        fdList[make_fd(s_sigpipe[0], FD_RD)] = NULL;

        // the read fd must not block
        fcntl(s_sigpipe[0], F_SETFL, O_NONBLOCK);
	
        // install signal handlers
        signal(SIGINT, sigint_handler);
        signal(SIGTERM, sigint_handler);
        signal(SIGUSR1, sigusr1_handler);
        signal(SIGALRM, sigalarm_handler);
        // FIXME sighup for log file rotation
				
        auto_ptr<CommandLineArgs> _args(new CommandLineArgs());
        args = _args;

        // basic args
        args->add('c', "ConfigFile", "<file>", "use alternative config file",
                  "MAIN", "configfile");
        args->add('l', "LogFile", "<file>", "use alternative log file",
                  "MAIN", "logfile");
        args->add('r', "BidFile", "<file>", "use specified bid file",
                  "MAIN", "BidFile");
        args->addFlag('V', "RelVersion", "show version info and exit",
                      "MAIN", "version");
        args->add('D', "FieldDefFile", "<file>", "use alternative file for field definitions",
                  "MAIN", "fdeffile");
        args->add('C', "FieldConstFile", "<file>", "use alternative file for field constants",
                  "MAIN", "fcontfile");
                  
#ifdef USE_SSL
        args->addFlag('x', "UseSSL", "use SSL for control communication",
                      "CONTROL", "usessl");
#endif

        // parse command line args
        if (args->parseArgs(argc, argv)) {
            // user wanted help
            exit(0);
        }

        if (args->getArgValue('V') == "yes") {
            cout << getHelloMsg();
            exit(0);
        }

        // test before registering the exit function
        if (alreadyRunning()) {
            throw Error("already running on this machine - terminating" );
        }

        // register exit function
        atexit(exit_fct);
				
        auto_ptr<Logger> _log(Logger::getInstance(AGNT_DEFAULT_LOG_FILE)); 	
        log = _log;
        ch = log->createChannel("Agent");

        log->log(ch,"Initializing Agent system");
        log->log(ch,"Program executable = %s", argv[0]);
        log->log(ch,"Started at %s", noNewline(ctime(&startTime)));
				
        // parse config file
        configFileName = args->getArgValue('c');
        if (configFileName.empty()) { 
            // is no config file is given then use the default
            // file located in a relative location to the binary
            configFileName = AGNT_DEFAULT_CONFIG_FILE;
        }

        log->log(ch,"ConfigFile = %s", configFileName.c_str());

        auto_ptr<ConfigManager> _conf(new 
			ConfigManager(AGNT_CONFIGFILE_DTD, configFileName, argv[0]));
        conf = _conf;

        // merge into config
        conf->mergeArgs(args->getArgList(), args->getArgVals());
				
        // dont need this anymore
        CommandLineArgs *a = args.release();
        saveDelete(a);

        // use logfilename (in order of precedence):
        // from command line / from config file / hardcoded default

        // query command line for log file name
        logFileName = conf->getValue("LogFile", "MAIN");

        if (logFileName.empty()) {
            logFileName = AGNT_DEFAULT_LOG_FILE;
        }

        log->setDefaultLogfile(logFileName);

        // set logging vebosity level if configured
        string verbosity = conf->getValue("VerboseLevel", "MAIN");
        if (!verbosity.empty()) {
            log->setLogLevel( ParserFcts::parseInt( verbosity, -1, 4 ) );
        }
        
#ifdef DEBUG
        log->log(ch,"configfilename used is: '%s'", configFileName.c_str());
        log->dlog(ch,"------- startup -------" );
#endif

		// Verifies Addresses.
		bool useIPV6 = false;
		string _uIPV6, _sIPV6, _sIPV4;
		_uIPV6 = conf->getValue("UseIPv6", "CONTROL");
		if (ParserFcts::parseBool(_uIPV6) == 1){ 
			useIPV6 = true;
		}
		
		if (useIPV6){
			_sIPV6 = conf->getValue("LocalAddr-V6", "CONTROL");
			// Verifies that given address is actually a ipv6 Address;
			struct in6_addr in6 = ParserFcts::parseIP6Addr(_sIPV6);
		}
		else{
			_sIPV4 = conf->getValue("LocalAddr-V4", "CONTROL");
			// Verifies that given address is actually a ipv4 Address;
			struct in_addr in4 = ParserFcts::parseIPAddr(_sIPV4);
		}
		 
#ifdef DEBUG
        log->log(ch," UseIpv:%d, IPv6:%s, Ipv4:%s", 
					useIPV6, _sIPV6.c_str(), _sIPV4.c_str());
#endif

        // startup other core classes
        auto_ptr<AuctionTimer> _auct(AuctionTimer::getInstance());
        auct = _auct;
        
        auto_ptr<BidManager> _bidm(new BidManager(conf->getValue("FieldDefFile", "MAIN"),
                                                    conf->getValue("FieldConstFile", "MAIN")));
        bidm = _bidm;
        
        auto_ptr<AuctionManager> _aucm(new AuctionManager(conf->getValue("FieldDefFile", "MAIN"),
															conf->getValue("FieldConstFile", "MAIN") ));
        aucm = _aucm;
        
        auto_ptr<ResourceRequestManager> _rreqm(new ResourceRequestManager(conf->getValue("FieldDefFile", "MAIN"),
															conf->getValue("FieldConstFile", "MAIN") ));
        rreqm = _rreqm;
        
        
        auto_ptr<MAPIResourceRequestParser> _mrrp(new MAPIResourceRequestParser());
        mrrp = _mrrp;

        auto_ptr<SessionManager> _ssmp(new SessionManager());
        ssmp = _ssmp;
        
        auto_ptr<EventSchedulerAgent> _evnt(new EventSchedulerAgent());
        evnt = _evnt;


#ifdef DEBUG
		log->dlog(ch,"------- eventSchedulerAgent loaded-------" );
#endif
		
		string anslpConfFile = conf->getValue("AnslpConfFile", "MAIN");

#ifdef DEBUG
		log->dlog(ch,"Anslp client conf file:%s", anslpConfFile.c_str() );
#endif
											 
		auto_ptr<AnslpClient> _anslpc(new AnslpClient(anslpConfFile));
					
		anslpc = _anslpc;
#ifdef DEBUG
		log->dlog(ch,"------- anslp client loaded-------" );
#endif
        // Startup Processing Components

#ifdef ENABLE_THREADS

        auto_ptr<AgentProcessor> _proc(new AgentProcessor(conf.get(),
							    conf->isTrue("Thread",
									 "AGENT_PROCESSOR")));
        pprocThread = conf->isTrue("Thread", "AGENT_PROCESSOR");
#else
        
        auto_ptr<AgentProcessor> _proc(new AgentProcessor(conf.get(), 
									 conf->getValue("FieldDefFile", "MAIN"), 
									 0));
        pprocThread = 0;
		
        if (conf->isTrue("Thread", "AGENT_PROCESSOR") ) {
            log->wlog(ch, "Threads enabled in config file but executable is compiled without thread support");
        }
#endif
        proc = _proc;
        proc->mergeFDs(&fdList);

		readDefaultData();
		
		// setup initial resource requests.
		string rfn = conf->getValue("ResourceRequestFile", "MAIN");

        if (!rfn.empty()) {
			evnt->addEvent(new AddResourceRequestsEvent(rfn));
        }

        // disable logger threading if not needed
        if (!pprocThread ) {
            log->setThreaded(0);
        }

		enableCtrl = conf->isTrue("Enable", "CONTROL");

		if (enableCtrl) {
			// ctrlcomm can never be a separate thread
			auto_ptr<CtrlComm> _comm(new CtrlComm(conf.get(), 0));
			comm = _comm;
			comm->mergeFDs(&fdList);
		}

#ifdef DEBUG
        log->dlog(ch,"------- end Agent constructor -------" );
#endif


    } catch (Error &e) {
        if (log.get()) {
            log->elog(ch, e);
        }  
        throw e;
    }
}


/* ---------------------------- ~Agent ---------------------------- */

Agent::~Agent()
{
    // other objects are destroyed by their auto ptrs

	agentTemplateListIter_t iter;
	for (iter = agentTemplates.begin(); iter != agentTemplates.end(); ++iter){
		delete(iter->second);
	}

#ifdef DEBUG
		log->dlog(ch,"------- end shutdown -------" );
#endif
    
}

void 
Agent::readDefaultData(void)
{

#ifdef DEBUG
		log->dlog(ch,"Starting readDefaultData" );
#endif

	bool bresult;
	
	if (anslpc.get() != NULL)
	{
		string sourceAddr = anslpc->getLocalAddress(); 
		protlib::hostaddress source_addr;
		bresult = source_addr.set_ip(sourceAddr.c_str());
		if (bresult == false){
			throw Error("Invalid default source address given %s", sourceAddr.c_str());
		}
		else{
			defaultSourceAddr = sourceAddr;
		}
		
		uint32_t lifetime = anslpc->getInitiatorLifetime(); 
		if (lifetime > 0){
			defaultLifeTime = lifetime;
		}
		else{
			throw Error("Invalid default lifetime given %d", lifetime);
		}	
	}
	else{
		throw Error("Anslp Client configuration Manager not initialized");
	}
	
	if (conf.get() != NULL)
	{

		// Read the source port
		string sSourcePort = conf->getValue("DefaultSourcePort", "MAIN");
		uint16_t sPort = atoi(sSourcePort.c_str());
		if (sPort <= 0){
			throw Error("Invalid default source port number %s", sSourcePort.c_str());
		}
		else{
			defaultSourcePort = sPort;
		}
		
		string sDestinAddress = conf->getValue("DefaultDestinAddr", "MAIN");
		protlib::hostaddress receiver_addr;
		bresult = receiver_addr.set_ip(sDestinAddress.c_str());
		if (bresult == false){
			throw Error("Invalid default destination address given %s", sDestinAddress.c_str());
		}
		else{
			defaultDestinAddr = sDestinAddress;
		}

		// Read the destination port
		string sDestinationPort = conf->getValue("DefaultDestinPort", "MAIN");
		uint16_t dPort = atoi(sDestinationPort.c_str());
		if (dPort <= 0){
			throw Error("Invalid default destination port number %s", sDestinationPort.c_str());
		}
		else{
			defaultDestinPort = dPort;
		}

		// Read the protocol
		string sprotocol = conf->getValue("DefaultProtocol", "MAIN");
		uint8_t protocol = atoi(sprotocol.c_str());
		if (protocol <= 0){
			throw Error("Invalid default protocol number %s", sprotocol.c_str());
		}
		else{
			defaultProtocol = protocol;
		}
	}
	else {
		throw Error("Configuration Manager not initialized");
	}

#ifdef DEBUG
		log->dlog(ch,"Ending readDefaultData" );
#endif

} 


/* -------------------- getHelloMsg -------------------- */

string Agent::getHelloMsg()
{
    ostringstream s;
    
    static char name[128] = "\0";

    if (name[0] == '\0') { // first time
        gethostname(name, sizeof(name));
    }

    s << "Agent build " << BUILD_TIME 
      << ", running at host \"" << name << "\"," << endl
      << "compile options: "
#ifndef ENABLE_THREADS
      << "_no_ "
#endif
      << "multi-threading support, "
#ifndef USE_SSL
      << "_no_ "
#endif
      << "secure sockets (SSL) support"
      << endl;
    
    return s.str();
}


/* -------------------- getInfo -------------------- */

string Agent::getInfo(agentInfoType_t what, string param)
{  
    time_t uptime;
    ostringstream s;
    
    s << "<info name=\"" << AgentManagerInfo::getInfoString(what) << "\" >";

    switch (what) {
    case AGI_AGENTMANAGER_VERSION:
        s << getHelloMsg();
        break;
    case AGI_UPTIME:
      uptime = ::time(NULL) - startTime;
        s << uptime << " s, since " << noNewline(ctime(&startTime));
        break;
    case AGI_BIDS_STORED:
        s << bidm->getNumBids();
        break;
    case AGI_CONFIGFILE:
        s << configFileName;
        break;
    case AGI_USE_SSL:
        s << (httpd_uses_ssl() ? "yes" : "no");
        break;
    case AGI_HELLO:
        s << getHelloMsg();
        break;
    case AGI_BIDLIST:
        s << CtrlComm::xmlQuote(bidm->getInfo());
        break;
    case AGI_BID:
        if (param.empty()) {
            throw Error("get_info: missing parameter for bid = <bidname>" );
        } else {
            int n = param.find(".");
            if (n > 0) {
                s << CtrlComm::xmlQuote(bidm->getInfo(param.substr(0,n), param.substr(n+1, param.length())));
            } else {
                s << CtrlComm::xmlQuote(bidm->getInfo(param));
            }
        }
        break;
    case AGI_NUMAGENTMANAGERINFOS:
    default:
        return string();
    }

    s << "</info>" << endl;
    
    return s.str();
}


string Agent::getAgentManagerInfo(agentInfoList_t *i)
{
    ostringstream s;
    agentInfoListIter_t iter;
   
    s << "<AgentManagerInfos>\n";

    for (iter = i->begin(); iter != i->end(); iter++) {
        s << getInfo(iter->type, iter->param);
    }

    s << "</AgentManagerInfos>\n";

    return s.str();
}


void Agent::handleGetInfo(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"Starting event Get info" );
#endif

	// get info types from event
    try {
		agentInfoList_t *i = ((GetInfoEvent *)e)->getInfos(); 
        // send meter info
        comm->sendMsg(getAgentManagerInfo(i), 
					  ((GetInfoEvent *)e)->getReq(), 
							fds, 0 /* do not html quote */ );
							
    } catch(Error &err) {
        comm->sendErrMsg(err.getError(), ((GetInfoEvent *)e)->getReq(), fds);
    }

#ifdef DEBUG
	log->dlog(ch,"Ending event Get info" );
#endif
}


void Agent::handleAddBids(Event *e, fd_sets_t *fds)
{
	bidDB_t *new_bids = NULL;

    try {

#ifdef DEBUG
       log->dlog(ch,"processing event adding bids" );
#endif
       // support only XML rules from file
       new_bids = bidm->parseBids(((AddBidsEvent *)e)->getFileName());
             
       // TODO AM : test rule spec 
       //proc->checkRules(new_rules);

#ifdef DEBUG
       log->dlog(ch,"Bids sucessfully checked " );
#endif

       // no error so lets add the rules and schedule for activation
       // and removal
       bidm->addBids(new_bids, evnt.get());

#ifdef DEBUG
       log->dlog(ch,"Bids sucessfully added " );
#endif

       saveDelete(new_bids);

	   /*
	    * If bid addition shall be performed _immediately_
		  (fds == NULL) then we need to execute this
		  activation event _now_ and not wait for the
		  EventScheduler to do this some time later.
	   */
	   if (fds == NULL ) {
		  Event *e = evnt->getNextEvent();
		  handleEvent(e, NULL);
		  saveDelete(e);
	   }

    } catch (Error &e) {
       // error in rule(s)
       if (new_bids) {
            saveDelete(new_bids);
       }
       throw e;
    }
}

void Agent::handleAddBidsAuction(Event *e)
{
#ifdef DEBUG
	log->dlog(ch,"processing event add bid auction" );
#endif		  
	try
	{

		Bid *b = ((InsertBidAuctionEvent *) e)->getBid();

		string auctionSet = ((InsertBidAuctionEvent *) e)->getAuctionSet();          
		string auctionName = ((InsertBidAuctionEvent *) e)->getAuctionName();
                    
		proc->addBidAuction(auctionSet, auctionName, b);
	}
	catch (Error &err) {
		log->dlog( ch, err.getError().c_str() );
	}
}

void Agent::handleRemoveBids(Event *e)
{
#ifdef DEBUG
	log->dlog(ch,"Starting event remove bids" );
#endif
    bidDB_t *bids = ((RemoveBidsEvent *)e)->getBids();
	  	  
    // now get rid of the expired bid
    proc->delBids(bids);
    bidm->delBids(bids, evnt.get());
    
#ifdef DEBUG
	log->dlog(ch,"Ending event remove bids" );
#endif
}

void Agent::handleRemoveBidFromAuction(Event *e)
{
#ifdef DEBUG
	log->dlog(ch,"Starting event remove bid from auction" );
#endif
	try{
		
		// Remove the link established between the bid and the auction.
		Bid *b = ((RemoveBidAuctionEvent *) e)->getBid();
        string auctionSet = ((RemoveBidAuctionEvent *) e)->getAuctionSet();
        string auctionName = ((RemoveBidAuctionEvent *) e)->getAuctionName();
        proc->delBidAuction(auctionSet, auctionName, b);
        bidm->delBid(b->getBidSet(), b->getBidName(), evnt.get());
        
    } catch(Error &err) {
		log->dlog( ch, err.getError().c_str() );
	}  
		
#ifdef DEBUG
        log->dlog(ch,"Ending event remove bid from auction" );
#endif
}

void Agent::handlePushExecution(Event *e, fd_sets_t *fds)
{
#ifdef DEBUG
	log->dlog(ch,"Starting push execution " );
#endif

              
#ifdef DEBUG
	log->dlog(ch,"Ending Push Execution" );
#endif
	
}

void Agent::handleAddAuctions(Event *e, fd_sets_t *fds)
{
	auctionDB_t *new_auctions = NULL;

    try {

#ifdef DEBUG
        log->dlog(ch,"processing event adding auctions" );
#endif

        // Put the auctions in the default domain, which is not valid.
        int domainId = 0;
        
        // Search the domain in the template container
        agentTemplateListIter_t iter = agentTemplates.find(domainId);
        if(iter == agentTemplates.end()){
			agentTemplates[domainId] = new ipap_template_container();
		}
		iter = agentTemplates.find(domainId);

        // support only XML rules from file
        new_auctions = aucm->parseAuctions(((AddAuctionsEvent *)e)->getFileName(), iter->second);

#ifdef DEBUG
        log->dlog(ch,"Auctions sucessfully parsed " );
#endif
             
        // no error so lets add the rules and schedule for activation
        // and removal
		aucm->addAuctions(new_auctions, evnt.get());

#ifdef DEBUG
        log->dlog(ch,"Auctions sucessfully added " );
#endif


        saveDelete(new_auctions);

		/*
		 * above 'addAuctions' produces an AuctionActivation event.
		 * If rule addition shall be performed _immediately_
		 (fds == NULL) then we need to execute this
		 activation event _now_ and not wait for the
		 EventScheduler to do this some time later.
		*/
		if (fds == NULL ) {
			Event *e = evnt->getNextEvent();
			handleEvent(e, NULL);
			saveDelete(e);
	    }

    } catch (Error &e) {
        // error in auctions(s)
        if (new_auctions) {
             saveDelete(new_auctions);
        }
        throw e;
    }
}

void Agent::handleActivateAuctions(Event *e)
{
#ifdef DEBUG
	log->dlog(ch,"Starting event activate auctions" );
#endif

    auctionDB_t *auctions = ((ActivateAuctionsEvent *)e)->getAuctions();

    // Add the auction to the process, so in that way we say that it is active.
    proc->addAuctions(auctions, evnt.get());

#ifdef DEBUG
	log->dlog(ch,"Ending event activate auctions" );
#endif
}

void Agent::handleRemoveAuctions(Event *e)
{	
	
	auctionDB_t *auctions = ((RemoveAuctionsEvent *)e)->getAuctions();

	// Delete the bids associated with all auctions.
	auctionDBIter_t  iter;
	for (iter = auctions->begin(); iter != auctions->end(); iter++) 
	{
		Auction *auct = *iter;
		vector<int> bidList = bidm->getBids(auct->getSetName(), 
											auct->getAuctionName());
											
		vector<int>::iterator bidListIter;
		for (bidListIter = bidList.begin(); 
						   bidListIter != bidList.end(); ++bidListIter)
		{
			Bid * bid = bidm->getBid(*bidListIter);
			
			evnt->addEvent(new RemoveBidAuctionEvent(bid, bid->getAuctionSet(), bid->getAuctionName() ));
		}
	}
		
	// Remove the auction from the processor
	proc->delAuctions(auctions);
	
	// Remove the auction from the manager
	aucm->delAuctions(auctions, evnt.get());
	
}

void Agent::handleAddResourceRequests(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"Processing add resource request" );
#endif
	resourceRequestDB_t *new_requests = NULL;

   	try{
		// Read the resource request from the given xml file

        // support only XML resource request from file
        new_requests = rreqm->parseResourceRequests(((AddResourceRequestsEvent *)e)->getFileName());
        
        // no error so lets add the resource requests, 
        // also schedule them for activation and removal
        rreqm->addResourceRequests(new_requests, evnt.get());
		
		// Activates resource request.
		/*
		 * The above 'addResourceRequests' produces an 
		 * ResourceRequestActivation event.
		 * 
		 * If Resource request addition shall be performed 
		 * _immediately_ (fds == NULL), then we need to execute this
		 * activation event _now_ and not wait for the EventScheduler 
		 * to do this some time later.
		*/
		if (fds == NULL ) {
			Event *e = evnt->getNextEvent();
			handleEvent(e, NULL);
			saveDelete(e);
		}
			  
        saveDelete(new_requests);


   } catch (Error &err) {
        // error in resource request(s)
        if (new_requests) {
            saveDelete(new_requests);
		}
        throw err;
   }
      
#ifdef DEBUG
   log->dlog(ch,"Ending add Resource request " );
#endif

}

void Agent::handleActivateResourceRequestInterval(Event *e)
{
#ifdef DEBUG
	log->dlog(ch,"Start event activate resource request interval" );
#endif

	ipap_message *mes = NULL;
	auction::Session *session = NULL;
	
	try{

		time_t start = ((ActivateResourceRequestIntervalEvent *)e)->getStartTime();
		resourceRequestDB_t *request = 
				((ActivateResourceRequestIntervalEvent *)e)->getResourceRequests();
				
		resourceRequestDBIter_t iter;
		for (iter = request->begin(); iter != request->end(); ++iter)
		{
			ResourceRequest *req = *iter;
			
			// Build the recordId as the resourceRequestSet + resourceRequestName
			string recordId = req->getResourceRequestSet() + "." + req->getResourceRequestName();
			
			/* TODO AM: for now we request all resources, 
			 * 			resource management must be implemented */ 
			string resourceId = "ANY"; 
			
			resourceReq_interval_t interval = req->getIntervalByStart(start);

			bool useIPV6 = false;
			string _uIPV6, _sIPV6, _sIPV4;
			_uIPV6 = conf->getValue("UseIPv6", "CONTROL");
			if (ParserFcts::parseBool(_uIPV6) == 1){ 
				useIPV6 = true;
			}
		
			if (useIPV6){
				_sIPV6 = conf->getValue("LocalAddr-V6", "CONTROL");
			}
			else{
				_sIPV4 = conf->getValue("LocalAddr-V4", "CONTROL");
			}

			string sPort = conf->getValue("ControlPort", "CONTROL");
			int port = atoi(sPort.c_str());
				  
			// Get the auctions corresponding with this resource request interval
			mes = mrrp->get_ipap_message(proc->getFieldDefinitions(), 
										 recordId,resourceId,interval,
										 useIPV6, _sIPV4, _sIPV6, port);
			
			// Add to the index of sessions to resource request intervals.
			
			session = req->getSession( interval.start, interval.stop, 
									   defaultSourceAddr, defaultSourceAddr, 
									   defaultDestinAddr, defaultSourcePort, 
									   defaultDestinPort, defaultProtocol,
									   defaultLifeTime  );
			
			// Convert the request to xml
			anslp::msg::anslp_ipap_message message(*mes);
			anslp::msg::anslp_ipap_xml_message xmlMes;
			string xmlMessage3 = xmlMes.get_message(message);
#ifdef DEBUG
			log->dlog(ch,xmlMessage3.c_str() );
#endif


			
			
			// Call the anslp client for sending the message.
			
			anslp::session_id sid = anslpc->tg_create( session->getSenderAddress(), 
											   session->getReceiverAddress(), 
											   session->getSenderPort(),
											   session->getReceiverPort(),
											   session->getProtocol(),
											   session->getLifetime(),
											   mes );
			session->setAnlspSession(sid);
			
			// Store the session as new in the sessionManager
			ssmp->addSession(session);
						
			// free the memory assigned.
			saveDelete(mes);
				  
		}

	#ifdef DEBUG
		log->dlog(ch,"Ending event activate resource request interval" );
	#endif
	
	} catch(Error &err) {
        // error in resource request(s)
        if (mes) {
            saveDelete(mes);
		}
		if (session){
			ssmp->delSession(session, evnt.get());
			saveDelete(session);
		}	
			
        throw err;
    }
}

void Agent::handleRemoveResourceRequestInterval(Event *e)
{
#ifdef DEBUG
	log->dlog(ch,"Starting event remove resource request interval" );
#endif

	time_t start = ((RemoveResourceRequestIntervalEvent *)e)->getStartTime();
	resourceRequestDB_t *request = 
				((RemoveResourceRequestIntervalEvent *)e)->getResourceRequests();

	auctionDB_t *auctionDb = NULL;

	try{

		resourceRequestDBIter_t iter;
		for (iter = request->begin(); iter != request->end(); ++iter)
		{
			
			ResourceRequest *req = *iter;
			// Get the auctions corresponding with this resource request interval
			string sessionId = req->getSession(start);
			
			AgentSession *session = reinterpret_cast<AgentSession *>(ssmp->getSession(sessionId));
			auctionSet_t auctions = session->getAuctions();

			// Add a new reference to the auction (there is another session reference it).
			aucm->decrementReferences(auctions, sessionId);
			
			auctionDb = new auctionDB_t();
			
			auctionSetIter_t iterAuctions;
			for (iterAuctions = auctions.begin(); iterAuctions != auctions.end(); ++iterAuctions)
			{
				if (((aucm->getAuction(*iterAuctions))->getSessionReferences()) == 0){
					auctionDb->push_back(aucm->getAuction(*iterAuctions));
				}
			}
			
			if (auctionDb->size() > 0 )
				// Remove auctions associated  with the resource interval
				evnt->addEvent(new RemoveAuctionsEvent(*auctionDb));
			
			// Delete pointers to auction objects.
			saveDelete(auctionDb);	
		}
	} catch (Error &err) {
		if (auctionDb){
			saveDelete(auctionDb);
		}
		
		log->dlog( ch, err.getError().c_str() );	
	}
#ifdef DEBUG
	log->dlog(ch,"Ending event remove resource request interval" );
#endif
	
}

void Agent::handleResponseCreateSession(Event *e, fd_sets_t *fds)
{
#ifdef DEBUG
	log->dlog(ch,"Starting event handleResponseCreateSession" );
#endif

	ipap_message *message = NULL;
	auctionDB_t *auctions = NULL;
	int domainId;

	try
	{

		string sessionId = ((ResponseCreateSessionEvent *)e)->getSession();
		
		// Obtains the message 
		message = ((ResponseCreateSessionEvent *)e)->getMessage();

		// Verifies if the domain is already in the agent template list
		domainId = message->get_domain();
		if (domainId > 0){
		
			agentTemplateListIter_t tmplIter;
			tmplIter = agentTemplates.find(domainId);
			if  (tmplIter == agentTemplates.end()){
				agentTemplates[domainId] = new ipap_template_container();
			} 
			
			tmplIter = agentTemplates.find(domainId);
				
			auctions = aucm->parseAuctionsMessage( message, tmplIter->second);
					
			// insert auctions in container 
			aucm->addAuctions(auctions, evnt.get());
			
			// Bring the id of every auction in the auctionDB.
			auctionSet_t setAuc; 
			aucm->getIds(auctions, setAuc);
			
			// Bring a reference to the session
			AgentSession *session = reinterpret_cast<AgentSession *>(ssmp->getSession(sessionId));
			
			// Add a new reference to the auction (there is another session reference it).
			aucm->incrementReferences(setAuc, sessionId );
			
			// Add related auctions to session object.
			session->setAuctions(setAuc);
			
			// delete the pointer to auctionDB.
			saveDelete(auctions);
			
			comm->sendMsg("", ((ResponseCreateSessionEvent *)e)->getReq(), fds); 

#ifdef DEBUG
			log->dlog(ch,"Ending event handleResponseCreateSession - auctions number:%d", auctions->size() );
#endif
			
		} else{
			throw Error("Agent: Invalid domain id associated with the message");
		}
	}
	catch (Error &err){
		if (auctions){
			saveDelete(auctions);
		}
		
		log->dlog( ch, err.getError().c_str() );	
		comm->sendErrMsg(err.getError(), ((ResponseCreateSessionEvent *)e)->getReq(), fds); 
	}

}

/* -------------------- handleEvent -------------------- */

void Agent::handleEvent(Event *e, fd_sets_t *fds)
{
   
    switch (e->getType()) {
    case TEST:
      {

#ifdef DEBUG
        log->dlog(ch,"processing event test" );
#endif

      }
      break;
    
    case GET_INFO:
		handleGetInfo(e,fds);
      break;
        
    case ADD_AUCTIONS:
		handleAddAuctions(e,fds);
      break;

    case ACTIVATE_AUCTIONS:
		handleActivateAuctions(e);
      break;

    case ADD_BIDS:
		handleAddBids(e,fds);
      break;
      
	case ADD_BID_AUCTION:
		handleAddBidsAuction(e);
	  break;
	
	case REMOVE_AUCTIONS:
		handleRemoveAuctions(e);
	  break;
	  
    case REMOVE_BIDS:
		handleRemoveBids(e);
      break;

	case REMOVE_BID_AUCTION:
		handleRemoveBidFromAuction(e);
	  break;

    case PUSH_EXECUTION:
		handlePushExecution(e,fds);
      break;

    case ADD_RESOURCEREQUESTS:
		handleAddResourceRequests(e,fds);
      break;
	    	   
    case ACTIVATE_RESOURCE_REQUEST_INTERVAL:
		handleActivateResourceRequestInterval(e);
      break;

    case REMOVE_RESOURCE_REQUEST_INTERVAL:
		handleRemoveResourceRequestInterval(e);
      break;

	case RESPONSE_CREATE_SESSION:
		handleResponseCreateSession(e, fds);
	  break;

    default:
#ifdef DEBUG
        log->dlog(ch,"Unknown event %s", eventNames[e->getType()].c_str() );
#endif
        throw Error("unknown event");
        break;
    }
}


/* ----------------------- run ----------------------------- */

void Agent::run()
{
    fdListIter_t   iter;
    fd_set         rset, wset;
    fd_sets_t      fds;
    struct timeval tv;
    int            cnt = 0;
    int            stop = 0;
    eventVec_t     retEvents;
    Event         *e = NULL;

    try {
        // fill the fd set
        FD_ZERO(&fds.rset);
        FD_ZERO(&fds.wset);
        for (iter = fdList.begin(); iter != fdList.end(); iter++) {
            if ((iter->first.mode == FD_RD) || (iter->first.mode == FD_RW)) {
                FD_SET(iter->first.fd, &fds.rset);
            }
            if ((iter->first.mode == FD_WT) || (iter->first.mode == FD_RW)) {
                FD_SET(iter->first.fd, &fds.wset);
            }
        }
        fds.max = fdList.begin()->first.fd;
		
		// register a timer for ctrlcomm (only online capturing)
		if (enableCtrl) {
		  int t = comm->getTimeout();
		  if (t > 0) {
				evnt->addEvent(new CtrlCommTimerEvent(t, t * 1000));
		  }
		}
		
		
        // start threads (if threading is configured)
        proc->run();

#ifdef DEBUG
        log->dlog(ch,"------- Agent Manager is running -------");
#endif

        do {
			// select
            rset = fds.rset;
            wset = fds.wset;
	    
			tv = evnt->getNextEventTime();

            // note: under most unix the minimal sleep time of select is
            // 10ms which means an event may be executed 10ms after expiration!
            if ((cnt = select(fds.max+1, &rset, &wset, NULL, &tv)) < 0) {
                 if (errno != EINTR) {
					throw Error("select error: %s", strerror(errno));
                 }
            }

            // check FD events
            if (cnt > 0)  {

#ifdef DEBUG			
				log->dlog(ch,"In check FD events");
#endif

                if (FD_ISSET( s_sigpipe[0], &rset)) {
                    // handle sig action
                    char c;
                    if (read(s_sigpipe[0], &c, 1) > 0) {
                        switch (c) {
                        case 'S':
                            stop = 1;
                            break;
                        case 'D':
                             cerr << *this;
                            break;
                        case 'A':
                            // next event
                       
                            // check Event Scheduler events
                            e = evnt->getNextEvent();

#ifdef DEBUG			
							log->dlog(ch,"Next Event %s", eventNames[e->getType()].c_str());
#endif     

                            if (e != NULL) {
                                // FIXME hack
                                if (e->getType() == CTRLCOMM_TIMER) {
                                    comm->handleFDEvent(&retEvents, NULL, 
                                                        NULL, &fds);
                                } else {
                                    handleEvent(e, &fds);
                                }
                                // reschedule the event
                                evnt->reschedNextEvent(e);
                                e = NULL;
                            }		    
                            break;
                        default:
                            throw Error("unknown signal");
                        } 
                        //} else {
                        //throw Error("sigpipe read error");
                    }
                } 
                else {
                    if (enableCtrl) {
                      cout << "enableCtrl" << endl;
                      comm->handleFDEvent(&retEvents, &rset, &wset, &fds);
                    }
                }
	        }	

            if (!pprocThread) {
				proc->handleFDEvent(&retEvents, NULL,NULL, NULL);
            }

            // schedule events
            if (retEvents.size() > 0) {
                for (eventVecIter_t iter = retEvents.begin();
                     iter != retEvents.end(); iter++) {

                    evnt->addEvent(*iter);
                }
                retEvents.clear(); 
            }
        } while (!stop);

		proc->waitUntilDone();

		log->log(ch,"NetAgent going down on Ctrl-C" );

#ifdef DEBUG
		log->dlog(ch,"------- shutdown -------" );
#endif


    } catch (Error &err) {
        
        cout << "error in run() method" << err << endl;
        if (log.get()) { // Logger might not be available yet
            log->elog(ch, err);
        }	   

        // in case an exception happens between get and reschedule event
        if (e != NULL) {
            saveDelete(e);
        }

        throw err;
    }
    catch (...){
		cout << "error in run() method" << endl;
	}
}

/* ------------------------- dump ------------------------- */

void Agent::dump(ostream &os)
{
    /* FIXME to be implemented */
    os << "dump" << endl;
}


/* ------------------------- operator<< ------------------------- */

ostream& auction::operator<<(ostream &os, Agent &obj)
{
    obj.dump(os);
    return os;
}

/* ------------------------ signal handler ---------------------- */

void Agent::sigint_handler(int i)
{
    char c = 'S';

    write(s_sigpipe[1], &c,1);
}

void Agent::sigusr1_handler(int i)
{
    char c = 'D';
    
    write(s_sigpipe[1], &c,1);
}

void Agent::exit_fct(void)
{
    unlink(AGNT_LOCK_FILE.c_str());
}

void Agent::sigalarm_handler(int i)
{
    g_timeout = 1;
}

/* -------------------- alreadyRunning -------------------- */

int Agent::alreadyRunning()
{
    FILE *file;
    char cmd[128];
    struct stat stats;
    int status, oldPid;

	cout << AGNT_LOCK_FILE.c_str() << endl;

    // do we have a lock file ?
    if (stat(AGNT_LOCK_FILE.c_str(), &stats ) == 0) { 
			
        // read process ID from lock file
        file = fopen(AGNT_LOCK_FILE.c_str(), "rt" );
        if (file == NULL) {
            throw Error("cannot open old pidfile '%s' for reading: %s\n",
                        AGNT_LOCK_FILE.c_str(), strerror(errno));
        }
        fscanf(file, "%d\n", &oldPid);
        fclose(file);

        // check if process still exists
        sprintf( cmd, "ps %d > /dev/null", oldPid );
        status = system(cmd);

        // if yes, do not start a new meter
        if (status == 0) {
            return 1;
        }

        // pid file but no meter process ->meter must have crashed
        // remove (old) pid file and proceed
        unlink(AGNT_LOCK_FILE.c_str());
    }
	
	cout << AGNT_LOCK_FILE.c_str() << endl;
	
    // no lock file and no running meter process
    // write new lock file and continue
    file = fopen(AGNT_LOCK_FILE.c_str(), "wt" );
    if (file == NULL) {
        throw Error("cannot open pidfile '%s' for writing: %s\n",
                    AGNT_LOCK_FILE.c_str(), strerror(errno));
    }
    
    cout << AGNT_LOCK_FILE.c_str() << endl;
    
    fprintf(file, "%d\n", getpid());
    fclose(file);

    return 0;
}


