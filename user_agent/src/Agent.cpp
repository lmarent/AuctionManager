
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
#include "anslp_ipap_exception.h" 


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
        log->log(ch,"------- startup -------" );
#endif

		// Initialize The openssl framework.
		anslp::init_framework();

        string _domainId = conf->getValue("Domain", "MAIN");
		domainId = ParserFcts::parseInt( _domainId );

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

        // setup the database connection string.
        string _dbIp = conf->getValue("DataBaseIpAddress", "MAIN");
        string _dbName = conf->getValue("DBname", "MAIN");
        string _dbUser = conf->getValue("DBUser", "MAIN");
        string _dbPassword = conf->getValue("DBPassword", "MAIN");
        string _dbPort = conf->getValue("DBPort", "MAIN");
        
        string connectionDb;
        if (!_dbName.empty()){
        
			connectionDb = "dbname=" + _dbName; 
			connectionDb = connectionDb + "user=" + _dbUser; 
			connectionDb = connectionDb + "password=" + _dbPassword;
			connectionDb = connectionDb + "hostaddr=" + _dbIp;
			connectionDb = connectionDb + "port=" + _dbPort;
		} 
        
        auto_ptr<BiddingObjectManager> _bidm(new BiddingObjectManager(domainId, 
																	  conf->getValue("FieldDefFile", "MAIN"),
																	  conf->getValue("FieldConstFile", "MAIN"),
																	  connectionDb));
        bidm = _bidm;
        
        auto_ptr<AuctionManager> _aucm(new AuctionManager(domainId, 
														  conf->getValue("FieldDefFile", "MAIN"),
														  conf->getValue("FieldConstFile", "MAIN"),
														  true));
        aucm = _aucm;
        
        auto_ptr<ResourceRequestManager> _rreqm(new ResourceRequestManager( domainId, 
															conf->getValue("FieldDefFile", "MAIN"),
															conf->getValue("FieldConstFile", "MAIN") ));
        rreqm = _rreqm;
        
        auto_ptr<AgentSessionManager> _asmp(new AgentSessionManager());
        asmp = _asmp;
        
        auto_ptr<EventSchedulerAgent> _evnt(new EventSchedulerAgent());
        evnt = _evnt;


#ifdef DEBUG
		log->log(ch,"------- eventSchedulerAgent loaded-------" );
#endif
													 
        // Startup Processing Components

#ifdef ENABLE_THREADS

        auto_ptr<AgentProcessor> _proc( new AgentProcessor(domainId,  
											conf.get(),
											conf->getValue("FieldDefFile", "MAIN"),
											conf->getValue("FieldConstFile", "MAIN"), 
											conf->isTrue("Thread","AGENT_PROCESSOR")
									   ));
									   
        pprocThread = conf->isTrue("Thread", "AGENT_PROCESSOR");
#else
        
        auto_ptr<AgentProcessor> _proc(new AgentProcessor(domainId,  
										conf.get(), 
										conf->getValue("FieldDefFile", "MAIN"),
										conf->getValue("FieldConstFile", "MAIN"),  
										0
									 ));
        pprocThread = 0;
		
        if (conf->isTrue("Thread", "AGENT_PROCESSOR") ) {
            log->wlog(ch, "Threads enabled in config file but executable is compiled without thread support");
        }
#endif
        proc = _proc;
        proc->mergeFDs(&fdList);

#ifdef ENABLE_THREADS

        auto_ptr<AnslpProcessor> _anslproc( new AnslpProcessor(conf.get(),
											conf->isTrue("Thread","ANSLP_PROCESSOR")
									   ));
									   
        aprocThread = conf->isTrue("Thread", "ANSLP_PROCESSOR");
#else
        
        auto_ptr<AnslpProcessor> _anslproc(new AnslpProcessor(conf.get(), 0 ));
        aprocThread = 0;
		
        if (conf->isTrue("Thread", "ANSLP_PROCESSOR") ) {
            log->wlog(ch, "Threads enabled in config file but executable is compiled without thread support");
        }
#endif

//#ifdef DEBUG
		log->log(ch,"Anslp Processor threaded: %d", aprocThread );
//#endif

        anslproc = _anslproc;
        anslproc->mergeFDs(&fdList);

		string anslpConfFile = conf->getValue("AnslpConfFile", "MAIN");

#ifdef DEBUG
		log->log(ch,"Anslp client conf file:%s", anslpConfFile.c_str() );
#endif
		auto_ptr<AnslpClient> _anslpc(new AnslpClient(anslpConfFile, anslproc->get_fqueue()));
					
		anslpc = _anslpc;
#ifdef DEBUG
		log->log(ch,"------- anslp client loaded-------" );
#endif

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

		// ctrlcomm can never be a separate thread
		auto_ptr<CtrlComm> _comm(new CtrlComm(conf.get(), 0));
		comm = _comm;
		comm->mergeFDs(&fdList);

#ifdef DEBUG
        log->log(ch,"------- end Agent constructor -------" );
#endif


    } catch (Error &e) {
        if (log.get()) {
            log->elog(ch, e);
        }  else {
			 cout << e.getError().c_str() << endl;
		}
        throw e;
    }
}


void Agent::handleRemoveSession(Event *e, fd_sets_t *fds)
{

//#ifdef DEBUG
    log->log(ch,"processing event remove session" );
//#endif

	anslp::objectList_t *objList = NULL;
	
	string sessionId;

	try {
		anslp::objectListIter_t it;
		
		sessionId = ((RemoveSessionEvent *)e)->getSessionId();
		objList = ((RemoveSessionEvent *)e)->getObjects();
				
	} catch(anslp::msg::anslp_ipap_bad_argument &e) {
		// The message was not parse, we dont have to do anything. 
		// We assumming that the sender will send the message again.
		throw Error(e.what());
	}
	
	anslp::objectList_t objListRet;
		
	try{
		// Remove the session from the container.
		asmp->delSession(sessionId, evnt.get());

	} catch(Error &e) {
		log->dlog(ch, e.getError().c_str() );
	}

    if (objList != NULL){
		
		anslp::objectListIter_t it;
		for (it = objList->begin(); it != objList->end(); ++it){
				
			anslp::mspec_rule_key key = it->first;
			anslp::anslp_ipap_message *ipap_mes = dynamic_cast<anslp::anslp_ipap_message *>(it->second);
			if (ipap_mes != NULL){
				objListRet.insert(std::pair<anslp::mspec_rule_key, 
										 anslp::msg::anslp_mspec_object *>(key,ipap_mes->copy()));
			}							 
		}
	} else {
		log->elog(ch, "The event does not have a valid list of objects");
	}

	// Confirm for the anslp application installed objects.
	anslpc->tg_remove( sessionId, objListRet);

}



/* ---------------------------- ~Agent ---------------------------- */

Agent::~Agent()
{
    // other objects are destroyed by their auto ptrs

	agentTemplateListIter_t iter;
	for (iter = agentTemplates.begin(); iter != agentTemplates.end(); ++iter){
		delete(iter->second);
	}

//#ifdef DEBUG
		log->log(ch,"------- end shutdown -------" );
//#endif
    
}

void 
Agent::readDefaultData(void)
{

#ifdef DEBUG
		log->log(ch,"Starting readDefaultData" );
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
		log->log(ch,"Ending readDefaultData" );
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
        s << bidm->getNumAuctioningObjects();
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
	log->log(ch,"Starting event Get info" );
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
	log->log(ch,"Ending event Get info" );
#endif
}


void Agent::handleAddResourceRequests(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"Processing add resource request" );
#endif

	auctioningObjectDB_t *new_requests = NULL;

   	try{
		// Read the resource request from the given xml file

        // support only XML resource request from file
        new_requests = rreqm->parseResourceRequests(((AddResourceRequestsEvent *)e)->getFileName());
        
        // no error so lets add the resource requests, 
        // also schedule them for activation and removal
        rreqm->addAuctioningObjects(new_requests, evnt.get());
		
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
        
        log->elog( ch, err.getError().c_str() );
        
        // error in resource request(s)
        if (new_requests) {
            saveDelete(new_requests);
		}

   }
      
#ifdef DEBUG
   log->dlog(ch,"Ending add Resource request " );
#endif

}


void Agent::handleAddResourceRequestsCntrlComm(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"Processing add resource request Cntrl Comm" );
#endif

	auctioningObjectDB_t *new_requests = NULL;

   	try{
		// Read the resource request from the given xml file
		string message = ((AddResourceRequestsCtrlCommEvent *)e)->getMessage();
        char *buf = strdup ( message.c_str() );
        new_requests = rreqm->parseResourceRequestsBuffer(buf, message.size(), 0);
        
        // no error so lets add the resource requests, 
        // also schedule them for activation and removal
        rreqm->addAuctioningObjects(new_requests, evnt.get());
		
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
        free(buf);

   } catch (Error &err) {
        
        log->elog( ch, err.getError().c_str() );
        
        // error in resource request(s)
        if (new_requests) {
            saveDelete(new_requests);
		}

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
	auction::AgentSession *session = NULL;
	bool insertedSession = false;
	
	time_t start = ((ActivateResourceRequestIntervalEvent *)e)->getStartTime();
	ResourceRequest *req = 
				((ActivateResourceRequestIntervalEvent *)e)->getResourceRequest();
				
	if (req != NULL){
						
		/* TODO AM: for now we request all resources, 
		 * 			resource management must be implemented */ 
		string resourceId = "ANY"; 
			
		resourceReqIntervalListIter_t interval = req->getIntervalByStart(start);

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
	
		try{
			
			// Get the auctions corresponding with this resource request interval
			mes = rreqm->get_ipap_message( req, start, resourceId, useIPV6, _sIPV4, _sIPV6, port);
				
			// Create a new session for sending the request with a temporary id. 
			// It is replaced with the one given by the anslp client
			
			session = asmp->createAgentSession( defaultSourceAddr, defaultSourceAddr, 
												defaultDestinAddr, defaultSourcePort, 
												defaultDestinPort, defaultProtocol,
												defaultLifeTime  );
			
			// Sets the session as not inserted in the container
			insertedSession = false;									
			uint32_t mid = session->getNextMessageId();
			mes->set_seqno(mid);
			mes->set_ackseqno(0);
												
			// establish the resource request information in the session.
			session->setRequestData(req->getSet(), req->getName());
										
			session->setStart(interval->start);
			session->setStop(interval->stop);
						
#ifdef DEBUG
			log->dlog(ch,"Anslp before tg_create" );
#endif
			
			// Call the anslp client for sending the message.
			anslpc->tg_create( session->getSessionId(),
							   session->getSenderAddress(), 
							   session->getReceiverAddress(), 
							   session->getSenderPort(),
							   session->getReceiverPort(),
							   session->getProtocol(),
							   session->getLifetime(),
							   *mes );

#ifdef DEBUG
			log->dlog(ch,"Anslp after tg_create" );
#endif
			
			
			// Add the message as pending for ack.
			session->addPendingMessage( *mes );
			
			// Store the session as new in the sessionManager
			asmp->addSession(session);
			insertedSession = true;

			// Assign the new session to the interval.
			interval->sessionId = session->getSessionId();
									
			// free the memory assigned.
			saveDelete(mes);

#ifdef DEBUG
	log->dlog(ch,"Ending event activate resource request interval" );
#endif


		} catch(Error &err) {
			
			log->elog( ch, err.getError().c_str() );
			
			// error in resource request(s)
			if (mes) {
				saveDelete(mes);
			}
			if (session){
				if (insertedSession){ 
					asmp->delSession(session, evnt.get());
				}
				saveDelete(session);
			}
			
			throw Error(err.getError().c_str());
		}
				  
	}

#ifdef DEBUG
	log->dlog(ch,"Ending event activate resource request interval" );
#endif
	
}

void Agent::handleActivateSession(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"Starting  event activate session" );
#endif
	string sessionId;
	string anslpSessionId;

	Session *ses = NULL;
	AgentSession *session = NULL; 

	try {
		anslp::objectListIter_t it;
		
		sessionId = ((ConfigureSessionEvent *)e)->getSessionId();
		anslpSessionId = ((ConfigureSessionEvent *)e)->getAnslpSession();
		
		ses = asmp->getSession(sessionId);

//#ifdef DEBUG
		log->log(ch,"Activate session %s - anslp sessionId: %s", 
							sessionId.c_str(), anslpSessionId.c_str() );
//#endif
		
		if (ses != NULL){
			session = reinterpret_cast<AgentSession*>(ses);
		
			// Set to the session the anslp session created by the client.
			session->setAnlspSession(anslpSessionId);
			asmp->indexActiveSession(sessionId, anslpSessionId); 
		
			session->setState(SS_ACTIVE); 
		
		} else {
			throw Error("Session %s not found in manager", sessionId.c_str());
		}
				
	} catch(Error &err) {
		// The message was not parse, we dont have to do anything. 
		// We assumming that the sender will send the message again.
		log->elog( ch, err.getError().c_str() );
	}

//#ifdef DEBUG
	log->log(ch,"Ending event activate session" );
//#endif

}

auctioningObjectDB_t * 
Agent::readAuctionList(ipap_message &message)
{

	auctioningObjectDB_t *auctions = NULL;
	
	try
	{
	

#ifdef DEBUG
		anslp::msg::anslp_ipap_message messagedebug(message);
		anslp::msg::anslp_ipap_xml_message xmlMesdebug;
		string xmlMessagedebug = xmlMesdebug.get_message(messagedebug);
		log->dlog(ch,xmlMessagedebug.c_str() );
#endif
		
		// Verifies if the domain is already in the agent template list
		domainId = message.get_domain();
		
		if (domainId > 0){
		
			agentTemplateListIter_t tmplIter;
			tmplIter = agentTemplates.find(domainId);
			if  (tmplIter == agentTemplates.end()){
				agentTemplates[domainId] = new ipap_template_container();
			} 
			
			tmplIter = agentTemplates.find(domainId);
				
			auctions = aucm->parseMessage( &message, tmplIter->second);
			
			if (auctions->size() == 0){		
				log->elog( ch, "Agent: Invalid group of auctions given in the message" );
				throw Error("Agent: Invalid group of auctions given in the message");
			} 
			
			return auctions;
			
		} else {
			throw Error("Agent: Invalid domain id associated with the message");
		}
	}
	catch (Error &err){
		
		if (auctions != NULL){
			for (auctioningObjectDBIter_t iter = auctions->begin(); iter != auctions->end(); iter++) {
				if (*iter != NULL) {
					// delete auction
					delete *iter;
				}
			} 
			saveDelete(auctions);
		}
					
        log->elog( ch, err.getError().c_str() );
		
		//TODO AM: implement return codes. 
		// for now it generates the error not including in the final event.
	}
}

void
Agent::intersectInterval( time_t startDttmAuc, time_t stopDttmAuc, 
							time_t startDttmReq, time_t stopDttmReq,
							  time_t &start, time_t &stop)
{

#ifdef DEBUG
	struct timeval t1, t2, t3, t4;
	t1.tv_sec = startDttmAuc;
	t2.tv_sec = stopDttmAuc;
	t3.tv_sec = startDttmReq;
	t4.tv_sec = stopDttmReq;
    log->dlog(ch,"Start intersect Interval %s - %s - %s -%s",  
						(Timeval::toString(t1)).c_str(), 
						 (Timeval::toString(t2)).c_str(),
						  (Timeval::toString(t3)).c_str(),
						   (Timeval::toString(t4)).c_str() );
#endif

	if (startDttmAuc <= startDttmReq) {		
		start = startDttmReq;
	} else {
		if (stopDttmReq <= startDttmAuc) {
			start = startDttmReq;
		} else {
			start = startDttmAuc;
		}
		
	}	
	
	if (stopDttmAuc <= stopDttmReq){
		if (startDttmReq >= stopDttmAuc){
			stop = stopDttmReq;
		} else {
			stop = stopDttmAuc;
		}
	} else {
		stop = stopDttmReq;
	}

#ifdef DEBUG
	struct timeval t5, t6;
	t5.tv_sec = start;
	t6.tv_sec = stop;
    log->dlog(ch,"Ending intersect Interval %s - %s",  
						  (Timeval::toString(t5)).c_str(),
						   (Timeval::toString(t6)).c_str() );
#endif
	
}



unsigned long 
Agent::createAuctions(auctioningObjectDB_t *auctions, 
					   resourceReqIntervalListIter_t &interval)
{

	auctioningObjectDB_t auctionsInsert;
	double bidIntervals = 0;
	double modulus = 0;

	// This variable maintains the auction with the maximal duration interval, so
	// The request should last at least an interval multiple. 
	unsigned long maxInterval = 0;
	
	// insert auctions in container ( this will trigger events to activate and remove)
	// First loop though the auctions to see if there is already an auction in the container, in that case bring the auction
	// from the container update the stop time if it greater that the previous time and destroy the new object.
	auctioningObjectDBIter_t auctIter;
	for ( auctIter = auctions->begin(); auctIter != auctions->end(); ++auctIter)
	{
		bidIntervals = 1;
		modulus = 0;
		Auction *a = dynamic_cast<Auction *>(*auctIter);
		Auction *a2 = aucm->getAuction(a->getSet(), a->getName());
		
		if (a2 != NULL){
			
			interval_t intervalAuct = a2->getInterval();
						
			if (a2->getStart() > interval->start){
				a2->setStart(interval->start);
			}
						
			if (a2->getStop() < interval->stop){
				a2->setStop(interval->stop);
				
				// Modify delete event for this auction.
				evnt.get()->rescheduleAuctionDelete(a2->getUId(), interval->stop);

			}	
			
			// Update the maximum interval for the auction.
			if (maxInterval < intervalAuct.interval) {
				maxInterval = intervalAuct.interval;
			}

			
			delete(a);
			auctions->erase(auctIter);
			auctions->push_back(a2);
			
		} else {
			
			interval_t intervalAuct = a->getInterval();			
			time_t start, stop;

			intersectInterval( a->getStart(), a->getStop(), 
								interval->start, interval->stop,
								 start, stop);
			
			a->setStart(start);
			a->setStop(stop);
			
			if (intervalAuct.interval > 0){
			  bidIntervals =  floor ( (a->getStop() - a->getStart()) / intervalAuct.interval );
			  modulus = (a->getStop() - a->getStart()) % intervalAuct.interval;
			} 	
			// If the requested time is less than the minimal interval for the auction, 
			// we have to request te minimal interval.
			if ( modulus > 0 ){
				time_t newStop = a->getStart() + 
									(intervalAuct.interval * (bidIntervals + 1));
				a->setStop(newStop);
			}

			// Update the maximum interval for the auction.
			if (maxInterval < intervalAuct.interval) {
				maxInterval = intervalAuct.interval;
			}
			
			auctionsInsert.push_back(a);
		}
	}
			
	aucm->addAuctioningObjects(&auctionsInsert, evnt.get());

	return maxInterval;
}

void 
Agent::createProcessRequests( AgentSession *session, 
							  auctioningObjectDB_t *auctions,
							  ResourceRequest *request,  
							  resourceReqIntervalListIter_t &interval, 
							  unsigned long maxInterval )
{

	double bidIntervals = 0;

	// Go through the list of auctions and create groups by their module 
	map<string, auctioningObjectDB_t> splitByModule;
	map<string, auctioningObjectDB_t>::iterator splitByModuleIter;
	string sModuleName;
	
	auctioningObjectDBIter_t auctIter;
	for ( auctIter = auctions->begin(); auctIter != auctions->end(); ++auctIter){
		// Read the name of the module to load
		Auction *aTmp = dynamic_cast<Auction *>(*auctIter);
		sModuleName = aTmp->getModuleName();
		splitByModuleIter = splitByModule.find(sModuleName);

		// Insert a pointer to the auction.
		(splitByModule[sModuleName]).push_back(aTmp);
	}
						
	time_t now = time(NULL);
	time_t req_start = interval->start;
	time_t req_end = interval->stop;

	// Create a process request for each group created.
	for (splitByModuleIter = splitByModule.begin(); splitByModuleIter != splitByModule.end();  ++splitByModuleIter)
	{	
								
		bool firstTime = true;
		int index = 0;
					
		auctioningObjectDBIter_t auctIter2;
		for ( auctIter2 = (splitByModuleIter->second).begin(); 
					auctIter2 != (splitByModuleIter->second).end(); ++auctIter2)
		{
			Auction *aTmp = dynamic_cast<Auction *>(*auctIter2);
				
			if (firstTime == true){

#ifdef DEBUG
			log->dlog(ch, "auction interval - start:%s stop:%s maxInterval:%lu", 
							Timeval::toString(interval->start).c_str(),
								Timeval::toString(interval->stop).c_str(), maxInterval);
#endif
				double modulus;
				if (maxInterval > 0)
				{
				  bidIntervals =  floor ( (req_end - req_start) / maxInterval );
				  modulus = (req_end - req_start) % maxInterval;
				  long duration = maxInterval * (bidIntervals + 1);
   				  req_end = req_start + duration;

#ifdef DEBUG
					 
				 log->dlog(ch, "new request time: %lu", duration);
#endif
				}
					
				// Create new request process for coming auctions.
				index = proc->addRequest( session->getSessionId(), 
										  request->getFields(), aTmp, 
										  req_start, req_end );
														
				// Insert the index in the resource process set created. 
				(interval->resourceProcesses).insert(index);
								
				firstTime = false;
			} else {
				proc->addAuctionRequest(index, aTmp );
			}
		}
										
		// Schedule the execution of the request.
		evnt.get()->addEvent(new PushExecutionEvent(req_start - now, index)); 

		// Schedule the delete of the request.
		evnt.get()->addEvent(new RemovePushExecutionEvent(req_end - now, index)); 
			
	}
}

void 
Agent::increaseSessionReferences(AgentSession *session, auctioningObjectDB_t *auctions )
{
	// Bring the id of every auction in the auctionDB.
	auctionSet_t setAuc; 
	aucm->getIds(auctions, setAuc);
								
	// Add a new reference to the auction (there is another session reference it).
	aucm->incrementReferences(setAuc, session->getSessionId() );
					
	// Add related auctions to session object.
	session->setAuctions(setAuc);
	
}

void
Agent::split( vector<string> & theStringVector,  /* Altered/returned value */
			  const  string  & theString,
			  const  string  & theDelimiter)
{
    assert( theDelimiter.size() > 0);

    size_t  start = 0, end = 0;

    while ( end != string::npos)
    {
        end = theString.find( theDelimiter, start);

        // If at end, use length=maxLength.  Else use length=end-start.
        theStringVector.push_back( theString.substr( start,
                       (end == string::npos) ? string::npos : end - start));

        // If at end, use start=maxSize.  Else use start=end+delimiter.
        start = (   ( end > (string::npos - theDelimiter.size()) )
                  ?  string::npos  :  end + theDelimiter.size());
    }
}

void 
Agent::replySessionCreationMessages( ipap_message &message, 
									 AgentSession *session, 
									 anslp::mspec_rule_key key, 
									 auctioningObjectDB_t *auctions, 
									 anslp::objectList_t *objectList,
									 std::vector<anslp::anslp_event_msg *> *events )
{
	
	string sipv4Address, sipv6Address, destinAddr;
	int iport, ipversion;
	set<string> connectStrings;
	
	// Get the address information of any of the auctions, so a reply can be sent.
	auctioningObjectDBIter_t iter;
	for ( iter = auctions->begin(); iter != auctions->end(); iter++){
		Auction *atmp = dynamic_cast<Auction *>(*iter);

		atmp->getConnectionString(sipv4Address, sipv6Address, 
									iport, ipversion, destinAddr);
		
		
		ostringstream streamTmp;
		streamTmp << sipv4Address << "%" << sipv6Address 
									<< "%" << iport << "%" << ipversion 
									<< "%" << destinAddr;
		
		connectStrings.insert(streamTmp.str());
	}
	
	set<string>::iterator iter2;
	for (iter2 = connectStrings.begin(); iter2 != connectStrings.end(); ++iter2)
	{
	    vector<string> separConnStr;
	    split( separConnStr, *iter2, "%");
	    
#ifdef DEBUG			
		log->dlog(ch, "handle create session - destin address:%s port:%d", destinAddr.c_str(), iport);
#endif
				
		// Build the response for the originator agent.
		ipap_message resp = ipap_message(domainId, IPAP_VERSION, true);
		resp.set_seqno(session->getNextMessageId());
		resp.set_ackseqno(message.get_seqno() + 1);
		resp.output();

#ifdef DEBUG
		log->dlog(ch,"Ending event handleResponseCreateSession - auctions number:%d sessionId:%s", 
					auctions->size(),session->getAnlspSession().c_str()  );
#endif
							
		// Finally send the message through the anslp client application.
		events->push_back(anslpc->delayed_tg_bidding( new anslp::session_id(session->getAnlspSession()), 
							session->getSenderAddress(), separConnStr[4], 
							session->getSenderPort(), atoi(separConnStr[2].c_str()) ,
							session->getProtocol(), 
							resp ));

		anslp::anslp_ipap_message ipap_mes_return(resp);		
		
		objectList->insert(std::pair<anslp::mspec_rule_key, 
							anslp::msg::anslp_mspec_object *>(key , ipap_mes_return.copy()));
	}
}

void Agent::handleSingleCreateSession(string sessionId, anslp::mspec_rule_key key, 
					anslp::anslp_ipap_message *ipap_mes, 
					anslp::objectList_t *objectList,
					std::vector<anslp::anslp_event_msg *> *events)
{

	int domainId;
	
	resourceReqIntervalListIter_t interval;
	ResourceRequest *request = NULL;
	Session *ses = NULL;
	AgentSession *session = NULL; 
	auctioningObjectDB_t *auctions = NULL;

#ifdef DEBUG
	log->dlog(ch,"Starting handleSingleCreateSession session id:%s", sessionId.c_str() );
#endif

	// The following lines bring the interval that creates the session.
	try{
		
		ses = asmp->getAnslpSession(sessionId);
		// Bring the session from that create the initial request.
		session = reinterpret_cast<AgentSession*>(ses);
		
		if (session == NULL){
			throw Error("Session Id:%s not found", sessionId.c_str());
		}	
		// Bring the request.
		request = rreqm->getResourceRequest(session->getResourceRequestSet(), 
											session->getResourceRequestName());
					
		// Bring the request interval.
		interval = request->getIntervalByStart(session->getStart());

#ifdef DEBUG			
		log->dlog(ch, "auction interval - start:%s stop:%s", 
							Timeval::toString(interval->start).c_str(),
								Timeval::toString(interval->stop).c_str());
#endif
				
	} catch(Error &err) {
				
		log->elog( ch, err.getError().c_str() );
		// TODO AM: Generate the error. For now no message means error. 
		throw Error(err.getError().c_str());
	}
	
	// So far, so good. we can proceed to check for session and request data.
	try 
	{	

		ipap_message message = ipap_mes->ip_message;

		uint32_t mid = message.get_ackseqno();
		
		// Acknowledge the message.
		session->confirmMessage(mid-1);
		
		auctions = readAuctionList(message);
	
		unsigned long maxInterval = createAuctions(auctions, interval);
	
		createProcessRequests( session, auctions, request, interval, maxInterval );
		
		increaseSessionReferences(session, auctions );
	
		replySessionCreationMessages(message, session, key, auctions, objectList, events);
		
	} catch (Error &err){
		
		log->elog( ch, err.getError().c_str() );
		
		if (auctions){
			saveDelete(auctions);
		}
		
		// TODO AM: Generate the error. For now no message means error. 
	}

#ifdef DEBUG
	log->dlog(ch,"Sucessfully ending handleSingleCreateSession" );
#endif

}

void Agent::handleResponseCreateSession(Event *e, fd_sets_t *fds)
{
//#ifdef DEBUG
	log->log(ch,"Starting event handleResponseCreateSession" );
//#endif



	anslp::objectList_t *objList = NULL;	
	string sessionId;
	
	try {
		anslp::objectListIter_t it;
		
		sessionId = ((CreateSessionEvent *)e)->getSessionId();
		objList = ((CreateSessionEvent *)e)->getObjects();
				
	} catch(anslp::msg::anslp_ipap_bad_argument &e) {
		// The message was not parse, we dont have to do anything. 
		// We assumming that the sender will send the message again.
		log->elog( ch, e.getError().c_str() );	
		return;
	}

//#ifdef DEBUG
	log->log(ch,"Starting event handleResponseCreateSession %s", sessionId.c_str() );
//#endif
	
	anslp::objectList_t objListRet;
	std::vector<anslp::anslp_event_msg *> events;
	
	try {
		if (objList != NULL){
			
			anslp::objectListIter_t it;
			for (it = objList->begin(); it != objList->end(); ++it){
					
				anslp::mspec_rule_key key = it->first;
				anslp::anslp_ipap_message *ipap_mes = dynamic_cast<anslp::anslp_ipap_message *>(it->second);
				if (ipap_mes != NULL){
					handleSingleCreateSession(sessionId, key, ipap_mes, &objListRet, &events);
				}		
			}
		} else {
			log->elog(ch, "The event does not have a valid list of objects");
		}
	
		log->log(ch,"Nbr objects installed %d ", objListRet.size() );
		
		// Confirm for the anslp application installed objects.
		anslpc->tg_install( sessionId, objListRet);
		
		// Send confirmation to every auction server involved.
		anslpc->tg_bidding(&events);

#ifdef DEBUG
	log->dlog(ch,"Ending event handleResponseCreateSession" );
#endif

	} catch(Error &err){
		
		log->elog( ch, err.getError().c_str() );
	}
}

void 
Agent::handlePushExecution(Event *e, fd_sets_t *fds)
{
#ifdef DEBUG
	log->dlog(ch,"Starting push execution " );
#endif
	
	try {
	
		// Get the index to execute 
		int index = ((PushExecutionEvent *)e)->getIndex();
		
		// Call the execution of te request process
		proc->executeRequest( index, evnt.get() );
		              
#ifdef DEBUG
		log->dlog(ch,"Ending Push Execution" );
#endif

	} catch (Error &err) {
		
		log->elog( ch, err.getError().c_str() );
	}
	
}

void 
Agent::handleRemovePushExecution(Event *e, fd_sets_t *fds)
{
#ifdef DEBUG
	log->dlog(ch,"Starting remove push execution " );
#endif

	try {
	
		// Get the index to execute 
		int index = ((RemovePushExecutionEvent *)e)->getIndex();
		
		// Call the execution of te request process
		proc->delRequest( index );
		              
#ifdef DEBUG
	log->dlog(ch,"Ending Remove Push Execution" );
#endif

	} catch (Error &err) {
		
		log->elog( ch, err.getError().c_str() );
	}
	
}


void 
Agent::handleAddBiddingObjects(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
    log->dlog(ch,"processing event adding bidding objects" );
#endif
       
    // NOTHING TO DO.

#ifdef DEBUG
    log->dlog(ch,"Bidding objects sucessfully added " );
#endif

}

void Agent::handleAddGeneratedBiddingObjects(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
    log->dlog(ch,"processing event add generated bidding objects" );
#endif

	auctioningObjectDB_t *new_bids = NULL;
	int index = 0;

   try {

       new_bids = ((AddGeneratedBiddingObjectsEvent *)e)->getBiddingObjects();
	   index = ((AddGeneratedBiddingObjectsEvent *)e)->getIndex();
	   
       // Add the new bidding object in the biddingObject manager
       bidm->addAuctioningObjects(new_bids, evnt.get());

	   evnt.get()->addEvent(new TransmitBiddingObjectsEvent(index, *new_bids));

#ifdef DEBUG
       log->dlog(ch,"BiddingObjects sucessfully added " );
#endif


    } catch (Error &e) {
       
       log->elog( ch, e.getError().c_str() );
    }	
	
}

void Agent::handleActivateBiddingObjects(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"Starting event Handle activate bidding Objects" );
#endif

	auctioningObjectDB_t *bids = NULL;

	try
	{    		
		
		bids = ((ActivateBiddingObjectsEvent *)e)->getBiddingObjects();
				
		bidm->activateAuctioningObjects(bids);
		
#ifdef DEBUG
		log->dlog(ch,"Ending event Handle activate bidding Objects" );
#endif		
	}
	catch (Error &err) {
		log->elog( ch, err.getError().c_str() );
	}

#ifdef DEBUG
	log->dlog(ch,"ending event Handle activate bidding Objects" );
#endif

}


void 
Agent::handleTransmitBiddingObjects(Event *e, fd_sets_t *fds)
{
	
#ifdef DEBUG
   log->dlog(ch,"starting handleTransmitBiddingObjects" );
#endif

	string sipv4Address, sipv6Address, destinAddr;
	int iport, ipversion;

	try{
		auctioningObjectDB_t *new_bids = ((TransmitBiddingObjectsEvent *)e)->getBiddingObjects();
		
		// get the request index 
		int index = ((TransmitBiddingObjectsEvent *)e)->getIndex();
		
		// Search for the corresponding session for this connection
		string sessionId = proc->getSession(index);
		AgentSession *session = reinterpret_cast<AgentSession *>(asmp->getSession(sessionId));
						
		auctioningObjectDBIter_t iter;
		for (iter = new_bids->begin(); iter != new_bids->end(); ++iter)
		{
			// We find the auction for the bid
			BiddingObject *biddingObject = dynamic_cast<BiddingObject *>(*iter);
			Auction *a = aucm->getAuction(biddingObject->getAuctionSet(), 
											biddingObject->getAuctionName());
			
			a->getConnectionString(sipv4Address, sipv6Address, 
										iport, ipversion, destinAddr);
													
			int domainAuct = ParserFcts::parseInt(a->getSet());

			// Search the domain in the template container
			agentTemplateListIter_t iterCont = agentTemplates.find(domainAuct);
			if(iterCont == agentTemplates.end()){
				throw Error("Auction domain:%d not found in templates container", domainAuct);
			}
			
			uint32_t mid = session->getNextMessageId();
			
			ipap_message *mes = bidm->get_ipap_message(biddingObject, a, iterCont->second);
			mes->set_seqno(mid);
			mes->set_ackseqno(0);
			
			// Save the message within the pending messages.
			session->addPendingMessage(*mes);
			
#ifdef DEBUG
			// Activate to see the bidding message to send.
			anslp::msg::anslp_ipap_xml_message messRes;
			anslp::msg::anslp_ipap_message ipapMesBid(*mes);
			string xmlMessage = messRes.get_message(ipapMesBid);
			log->dlog(ch,"Bidding message: %s", xmlMessage.c_str() );
#endif
			
			
			// Finally send the message through the anslp client application.
			anslpc->tg_bidding( new anslp::session_id(session->getAnlspSession()), 
								session->getSenderAddress(), destinAddr, 
								session->getSenderPort(), iport,
								session->getProtocol(), 
								*mes );
			
			saveDelete(mes);

		}
#ifdef DEBUG
		log->dlog(ch,"ending handleTransmitBiddingObjects" );
#endif
		
	} catch (Error &e){
		
		log->elog( ch, e.getError().c_str() );
		
	}
}

void Agent::handleAddAuctions(Event *e, fd_sets_t *fds)
{
	auctioningObjectDB_t *new_auctions = NULL;

    try {

#ifdef DEBUG
        log->dlog(ch,"processing event adding auctions" );
#endif

	   // NOTHING TO DO.

#ifdef DEBUG
        log->dlog(ch,"Auctions sucessfully added " );
#endif



    } catch (Error &e) {
        
        log->elog( ch, e.getError().c_str() );
        
        // error in auctions(s)
        if (new_auctions) {
             saveDelete(new_auctions);
        }

    }
}

void Agent::handleActivateAuctions(Event *e)
{

#ifdef DEBUG
	log->dlog(ch,"Starting event activate auctions" );
#endif

	try{

		auctioningObjectDB_t *auctions = ((ActivateAuctionsEvent *)e)->getAuctions();
		
		// set the status to active
		aucm->activateAuctioningObjects(auctions);
    
#ifdef DEBUG
		log->dlog(ch,"Ending event activate auctions" );
#endif

    } catch (Error &e) {
        
        log->elog( ch, e.getError().c_str() );
        
    }


}

void Agent::handleRemoveAuctions(Event *e)
{	
	
	try{
	
		// We assume that auctions have been already removed from the processor.	
		auctioningObjectDB_t *auctions = ((RemoveAuctionsEvent *)e)->getAuctions();
		
		// We remove the auction from all process requests.
		proc->delAuctions(auctions);
		
		// In the client application, we delete bids associated with all auctions.
		auctioningObjectDBIter_t  iter;
		
		for (iter = auctions->begin(); iter != auctions->end(); iter++) 
		{
			Auction *auct = dynamic_cast<Auction *>(*iter);
			vector<int> bidList = bidm->getBiddingObjects(auct->getSet(), 
														 auct->getName());
												
			auctioningObjectDB_t bids;
			vector<int>::iterator bidListIter;
			for (bidListIter = bidList.begin(); bidListIter != bidList.end(); ++bidListIter)
			{
				BiddingObject *bid = dynamic_cast<BiddingObject *>(bidm->getAuctioningObject(*bidListIter));
				if (bid->getType() == IPAP_BID){
					bids.push_back(bid);
				}
			}
			
			evnt->addEvent(new RemoveBiddingObjectsEvent(bids));
		}
				
		// Remove the auction from the manager
		aucm->delAuctioningObjects(auctions, evnt.get());

    } catch (Error &e) {
        
        log->elog( ch, e.getError().c_str() );
        
    }

	
}


void Agent::handleRemoveResourceRequestInterval(Event *e)
{
#ifdef DEBUG
	log->dlog(ch,"Starting event remove resource request interval" );
#endif

	time_t stop = ((RemoveResourceRequestIntervalEvent *)e)->getStopTime();
	
	ResourceRequest *request = 
				((RemoveResourceRequestIntervalEvent *)e)->getResourceRequest();

	try{

		if (request != NULL){

			auctioningObjectDB_t auctionDb;
			
			resourceReqIntervalListIter_t interval = (request)->getIntervalByEnd(stop);
			
			// Get the auctions corresponding with this resource request interval
			string sessionId = interval->sessionId;
			
			cout << "session inside the resource request interval:" << sessionId << endl;
						
			AgentSession *session = reinterpret_cast<AgentSession *>(asmp->getSession(sessionId));
		
			if (session != NULL){
				auctionSet_t auctions = session->getAuctions();
				
				cout << "session inside the resource request interval:" << sessionId << endl;
				
				// teardown the session created.
				anslpc->tg_teardown( new anslp::session_id(session->getAnlspSession())); 
				
				// Delete active request process associated with this request interval.
				set<int> requestProcs = interval->resourceProcesses;
				for ( set<int>::iterator it = requestProcs.begin(); it != requestProcs.end(); ++it){
					proc->delRequest( *it );
				}
				
				cout << "session inside the resource request interval 1:" << sessionId << endl;
				
				if (auctions.size() > 0){
					// delete the reference to the auction (a session is not referencing it anymore).
					aucm->decrementReferences(auctions, sessionId);
				}
				
				cout << "session inside the resource request interval 2:" << sessionId << endl;
							
				auctionSetIter_t iterAuctions;
				for (iterAuctions = auctions.begin(); iterAuctions != auctions.end(); ++iterAuctions)
				{
					Auction *auct = dynamic_cast<Auction *>(aucm->getAuctioningObject(*iterAuctions));
					if (auct != NULL){ // The auction was not previously removed.
						if (auct->getSessionReferences() == 0){
							auctionDb.push_back(auct);
						}
					}
				}
				
				if (auctionDb.size() > 0 )
					// Remove auctions associated  with the resource interval
					evnt->addEvent(new RemoveAuctionsEvent(auctionDb));
				
				cout << "session inside the resource request interval 3:" << sessionId << endl;
			}
			else {
				log->elog(ch,"Could not find the session for the request" );
			}
		}
		else {
			// Nothing to do
		}
#ifdef DEBUG
	log->dlog(ch,"Ending event remove resource request interval" );
#endif

	} catch (Error &err) {		
		log->elog( ch, err.getError().c_str() );	
	}
	
}

void Agent::handleRemoveBiddingObjects(Event *e)
{
#ifdef DEBUG
	log->dlog(ch,"Starting event remove bidding objects" );
#endif
	
	try{

		auctioningObjectDB_t *bids = ((RemoveBiddingObjectsEvent *)e)->getBiddingObjects();
			  
		// now get rid of the expired bid
		bidm->delAuctioningObjects(bids, evnt.get());
        
#ifdef DEBUG
	log->dlog(ch,"Ending event remove bidding objects" );
#endif

	} catch (Error &err) {		
		log->elog( ch, err.getError().c_str() );	
	}

}


void Agent::handleSingleObjectAuctioningInteraction(string sessionId, anslp::anslp_ipap_message *ipap_mes)
{

	auctioningObjectDB_t *bids = NULL;
	Session *ses = NULL;
	AgentSession *session = NULL; 
	string sipv4Address, sipv6Address, destinAddr;
	int iport, ipversion;
	

	try {

		assert(ipap_mes != NULL);	
		ipap_message message = ipap_mes->ip_message;

		// Search for the anslp session that is involved.
		ses = asmp->getAnslpSession(sessionId);		
		session = reinterpret_cast<AgentSession *>(ses);

		if (session == NULL)
			throw Error("Session %s not found", sessionId.c_str());
		
		// get the msgSeqNbr from the message
		uint32_t seqNbr = message.get_seqno();
		uint32_t ackSeqNbr = message.get_ackseqno();
		
		// send the same message arriving, if it is confirming a previous message. 
		if (ackSeqNbr > 0){
			session->confirmMessage(ackSeqNbr-1);
			
#ifdef DEBUG
			log->dlog(ch,"Ending handle Auction Interaction" );
#endif				

		} 
		else 
		{
		
			int domainBidObj = message.get_domain();
			// Search the domain in the template container
			agentTemplateListIter_t iterCont = agentTemplates.find(domainBidObj);
			if(iterCont == agentTemplates.end()){
				throw Error("Bidding Object domain:%d not found in templates container", domainBidObj);
			}
			
			bids = bidm->parseMessage(&message,iterCont->second);
			
			// Add the bidding objects to the bidding object manager.
			bidm->addAuctioningObjects(bids, evnt.get());  
																
			// If the number of bids is greater than zero, then take the first bidding object
			// to find the replying address.
			
			if ( bids->size() > 0 ){
				auctioningObjectDBIter_t iter = bids->begin();
				
				BiddingObject *biddingObject = dynamic_cast<BiddingObject *>(*iter);
				Auction *a = aucm->getAuction(biddingObject->getAuctionSet(), 
											biddingObject->getAuctionName());
				if (a == NULL){
					// the auction was recently deleted because of its stop time, so search in the stored objects
					a = dynamic_cast<Auction *>( aucm->getAuctioningObjectDone(biddingObject->getAuctionSet(), 
									biddingObject->getAuctionName()));
					if (a == NULL){
						throw Error("Auction :%s.%s was not found in the auction manager", 
										biddingObject->getAuctionSet().c_str(),
										biddingObject->getAuctionName().c_str());
					}
				}
				
				a->getConnectionString(sipv4Address, sipv6Address, 
										iport, ipversion, destinAddr);
																
				// Build the response for the originator agent.
				ipap_message conf = ipap_message(domainId, IPAP_VERSION, true);
				conf.set_seqno(session->getNextMessageId());
				conf.set_ackseqno(seqNbr+1);
				conf.output();
							
				// Finally send the message through the anslp client application.
				anslpc->tg_bidding( new anslp::session_id(session->getAnlspSession()), 
									session->getSenderAddress(), destinAddr, 
									session->getSenderPort(), iport,
									session->getProtocol(), 
									conf );
			
			}		
			
			bids->erase(bids->begin(), bids->end());
			saveDelete(bids);

			
#ifdef DEBUG
			log->dlog(ch,"Ending handle Auction Interaction" );
#endif
			
		}
	} catch (Error &err){
		log->elog( ch, err.getError().c_str() );
	}

}

void Agent::handleAuctioningInteraction(Event *e, fd_sets_t *fds)
{


	anslp::objectList_t *objList = NULL;
	string sessionId;
		
	try {
		
		sessionId = ((AuctionInteractionEvent *)e)->getSessionId();
		objList = ((AuctionInteractionEvent *)e)->getObjects();
				
	} catch(anslp::msg::anslp_ipap_bad_argument &e) {
		// The message was not parse, we dont have to do anything. 
		// We assumming that the sender will send the message again.
		throw Error(e.what());
    }
    
    try{

		if (objList != NULL){
			
			anslp::objectListIter_t it;
			for (it = objList->begin(); it != objList->end(); ++it){
				
				anslp::anslp_ipap_message *ipap_mes = dynamic_cast<anslp::anslp_ipap_message *>(it->second);
				if (ipap_mes != NULL)
					handleSingleObjectAuctioningInteraction(sessionId, ipap_mes);
				
			}
		} else {
			log->elog(ch, "The event does not have a valid list of objects");
		}
		
    } catch (Error &err) {
		log->elog( ch, err.getError().c_str() );	
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

	case REMOVE_AUCTIONS:
		handleRemoveAuctions(e);
	  break;

    case ADD_BIDDING_OBJECTS:
		handleAddBiddingObjects(e,fds);
      break;
	
	case ADD_GENERATED_BIDDING_OBJECTS:
		handleAddGeneratedBiddingObjects(e,fds);
	  break;

	case TRANSMIT_BIDDING_OBJECTS:
		handleTransmitBiddingObjects(e,fds);
	  break;
      
	case ACTIVATE_BIDDING_OBJECTS:
		handleActivateBiddingObjects(e,fds);
	  break;
		  
    case REMOVE_BIDDING_OBJECTS:
		handleRemoveBiddingObjects(e);
      break;

    case PUSH_EXECUTION:
		handlePushExecution(e,fds);
      break;
	
	case REMOVE_PUSH_EXECUTION:
		handleRemovePushExecution(e, fds);
	  break;
	  
    case ADD_RESOURCEREQUESTS:
		handleAddResourceRequests(e,fds);
      break;

    case ADD_RESOURCEREQUESTS_CTRLCOMM:
		handleAddResourceRequestsCntrlComm(e,fds);
      break;
	    	   
    case ACTIVATE_RESOURCE_REQUEST_INTERVAL:
		handleActivateResourceRequestInterval(e);
      break;

    case REMOVE_RESOURCE_REQUEST_INTERVAL:
		handleRemoveResourceRequestInterval(e);
      break;
	  
	case AUCTION_INTERACTION:
		 handleAuctioningInteraction(e, fds);
	  break;

	case CREATE_SESSION:
		handleResponseCreateSession(e, fds);
	  break;
	
	case CONFIGURE_SESSION:
		handleActivateSession(e,fds);
	  break;

    case REMOVE_SESSION:
		handleRemoveSession(e,fds);
		break;
	  
    default:
#ifdef DEBUG
        log->elog(ch,"Unknown event %s", eventNames[e->getType()].c_str() );
#endif
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

	protlib::log::DefaultLog.set_filter(DEBUG_LOG, LOG_CRIT);
	protlib::log::DefaultLog.set_filter(EVENT_LOG, LOG_CRIT);


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
		int t = comm->getTimeout();
		if (t > 0) {
			evnt->addEvent(new CtrlCommTimerEvent(t, t * 1000));
		}
				
        // start threads (if threading is configured)
        proc->run();

#ifdef DEBUG
        log->log(ch,"------- Agent Manager is running -------");
#endif

		// Establish the maximum timeout for waiting during select.
		struct timeval tv_anslp = {0,10000};

        do {
			// select
            rset = fds.rset;
            wset = fds.wset;
	    
			tv = evnt->getNextEventTime();

			// Calculates the min between the event timeout and the anslp queu timeout 
			if (Timeval::cmp(tv_anslp, tv) < 0) 
				tv = tv_anslp;
				
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
				log->dlog(ch,"In check FD events time:%s", (Timeval::toString(tv)).c_str());
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
#ifdef DEBUG			
									log->dlog(ch,"Next Event is a control timer");
#endif
                                    comm->handleFDEvent(&retEvents, NULL, NULL, &fds);
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
#ifdef DEBUG			
				   log->dlog(ch,"Next Event is not call for control but not because a control timer");
#endif					
                   comm->handleFDEvent(&retEvents, &rset, &wset, &fds);
                }
	        }	

            if (!pprocThread) {
				proc->handleFDEvent(&retEvents, NULL,NULL, NULL);
            }
			
			if (!aprocThread) {
				anslproc->handleFDEvent(&retEvents, NULL,NULL, NULL);
			}
			
			bool pendingExec = true;
			
			log->dlog(ch,"after handleFDEvent events: %d", retEvents.size());
			
            // schedule events
            if (retEvents.size() > 0) {
				
                for (eventVecIter_t iter = retEvents.begin();
                     iter != retEvents.end(); iter++) {
										
                    if (pendingExec){
						evnt->addEvent(*iter);
					}
                }
                retEvents.clear(); 
            }
        } while (!stop);

		proc->waitUntilDone();
		anslproc->waitUntilDone();

		anslp::cleanup_framework();

		log->log(ch,"NetAgent going down on Ctrl-C" );
		log->log(ch,"------- shutdown -------" );


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
	
    // no lock file and no running process
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
