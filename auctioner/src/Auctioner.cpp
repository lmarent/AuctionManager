
/*! \file   AuctionManager.cpp

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

    $Id: Auctioner.cpp 748 2015-03-05 11:01:01 amarentes $
*/


#include "ParserFcts.h"
#include "logfile.h"
#include "httpd.h"
#include "Auctioner.h"
#include "ConstantsAum.h"
#include "EventAuctioner.h"
#include "anslp_ipap_xml_message.h"
#include "anslp_ipap_message.h"

using namespace auction;

protlib::log::logfile commonlog("Auctioneer.log", anslp::anslp_config::USE_COLOURS);
protlib::log::logfile &protlib::log::DefaultLog(commonlog);

// globals in AuctionManager class
int Auctioner::s_sigpipe[2];
int Auctioner::enableCtrl = 0;

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


/* ------------------------- Auctioner ------------------------- */

Auctioner::Auctioner( int argc, char *argv[])
    :  domainId(0), pprocThread(0)
{

    // record auction manager start time for later output
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
                  "MAIN", "rulefile");
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
        args->add('P', "ControlPort", "<portnumber>", "use alternative control port",
                  "CONTROL", "cport");
#ifndef ENABLE_NF
#ifndef HAVE_LIBIPULOG_LIBIPULOG_H
        args->add('i', "NetInterface", "<iface>[,<iface2>,...]", "select network interface(s)"
                  " to capture from", "MAIN", "interface");
        args->addFlag('p', "NoPromiscInt", "don't put interface in "
                      "promiscuous mode", "MAIN", "nopromisc");
#endif
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
				
        auto_ptr<Logger> _log(Logger::getInstance()); 	
        log = _log;
        ch = log->createChannel("Auctioner");

        log->log(ch,"Initializing auctioner system");
        log->log(ch,"Program executable = %s", argv[0]);
        log->log(ch,"Started at %s", noNewline(ctime(&startTime)));
				
        // parse config file
        configFileName = args->getArgValue('c');
        if (configFileName.empty()) { 
            // is no config file is given then use the default
            // file located in a relative location to the binary
            configFileName = NETAUM_DEFAULT_CONFIG_FILE;
        }

        log->log(ch,"ConfigFile = %s", configFileName.c_str());

        auto_ptr<ConfigManager> _conf(new 
				ConfigManager(AUM_CONFIGFILE_DTD, configFileName, argv[0]));
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
            logFileName = AUM_DEFAULT_LOG_FILE;
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

        // Read parameters from configuration file.
        
        // set the domain id
        string _domainId = conf->getValue("Domain", "MAIN");
		domainId = ParserFcts::parseInt( _domainId );
		
		auctionerTemplates[domainId] = new ipap_template_container();

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
        log->log(ch,"domainId used is: '%d' UseIpv:%d, IPv6:%s, Ipv4:%s", 
					domainId, useIPV6, _sIPV6.c_str(), _sIPV4.c_str());
#endif

        // startup other core classes
        auto_ptr<AuctionTimer> _auct(AuctionTimer::getInstance());
        auct = _auct;
        
        auto_ptr<BiddingObjectManager> _bidm(new BiddingObjectManager(domainId, 
																	  conf->getValue("FieldDefFile", "MAIN"),
																	  conf->getValue("FieldConstFile", "MAIN")));
        bidm = _bidm;
        
        auto_ptr<AuctionManager> _aucm(new AuctionManager( domainId,
														   conf->getValue("FieldDefFile", "MAIN"),
														   conf->getValue("FieldConstFile", "MAIN")));
        aucm = _aucm;

        auto_ptr<SessionManager> _sesm(new SessionManager());
        sesm = _sesm;
        
        auto_ptr<EventSchedulerAuctioner> _evnt(new EventSchedulerAuctioner());
        evnt = _evnt;

		string anslpConfFile = conf->getValue("AnslpConfFile", "MAIN");

#ifdef DEBUG
		log->dlog(ch,"Anslp client conf file:%s", anslpConfFile.c_str() );
#endif
											 
		auto_ptr<AnslpClient> _anslpc(new AnslpClient(anslpConfFile));
					
		anslpc = _anslpc;
	
        // Startup Processing Components

#ifdef ENABLE_THREADS

        auto_ptr<AUMProcessor> _proc(new AUMProcessor( domainId, conf.get(),
													   conf->getValue("FieldDefFile", "MAIN"),
													   conf->getValue("FieldConstFile", "MAIN"),
													   conf->isTrue("Thread", "AUM_PROCESSOR"))
									);
									
        pprocThread = conf->isTrue("Thread", "AUM_PROCESSOR");
#else
        
        auto_ptr<AUMProcessor> _proc(new AUMProcessor( domainId, conf.get(), 
													   conf->getValue("FieldDefFile", "MAIN"), 
													   conf->getValue("FieldConstFile", "MAIN"),
														0)
									);
        pprocThread = 0;
		
		
        if (conf->isTrue("Thread", "AUM_PROCESSOR") ) {
            log->wlog(ch, "Threads enabled in config file but executable is compiled without thread support");
        }
#endif
        proc = _proc;
        proc->mergeFDs(&fdList);
		
		// setup initial auctions
		string rfn = conf->getValue("AuctionFile", "MAIN");

        if (!rfn.empty()) {
			evnt->addEvent(new AddAuctionsEvent(rfn));
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
        log->dlog(ch,"------- end Auctioner constructor -------" );
#endif


    } catch (Error &e) {
        if (log.get()) {
            log->elog(ch, e);
        }  
        throw e;
    }
}


/* ------------------------- ~Auctioner ------------------------- */

Auctioner::~Auctioner()
{
    // objects are destroyed by their auto ptrs
	
	auctionerTemplateListIter_t iter;
	for (iter = auctionerTemplates.begin(); iter != auctionerTemplates.end(); ++iter){
		delete(iter->second);
	}

#ifdef DEBUG
		log->dlog(ch,"------- end shutdown -------" );
#endif
    
}


/* -------------------- getHelloMsg -------------------- */

string Auctioner::getHelloMsg()
{
    ostringstream s;
    
    static char name[128] = "\0";

    if (name[0] == '\0') { // first time
        gethostname(name, sizeof(name));
    }

    s << "Auctioner build " << BUILD_TIME 
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

string Auctioner::getInfo(infoType_t what, string param)
{  
    time_t uptime;
    ostringstream s;
    
    s << "<info name=\"" << AuctionManagerInfo::getInfoString(what) << "\" >";

    switch (what) {
    case I_AUCTIONMANAGER_VERSION:
        s << getHelloMsg();
        break;
    case I_UPTIME:
      uptime = ::time(NULL) - startTime;
        s << uptime << " s, since " << noNewline(ctime(&startTime));
        break;
    case I_BIDS_STORED:
        s << bidm->getNumBiddingObjects();
        break;
    case I_CONFIGFILE:
        s << configFileName;
        break;
    case I_USE_SSL:
        s << (httpd_uses_ssl() ? "yes" : "no");
        break;
    case I_HELLO:
        s << getHelloMsg();
        break;
    case I_BIDLIST:
        s << CtrlComm::xmlQuote(bidm->getInfo());
        break;
    case I_BID:
        if (param.empty()) {
            throw Error("get_info: missing parameter for biddingObject = <biddingObjectName>" );
        } else {
            int n = param.find(".");
            if (n > 0) {
                s << CtrlComm::xmlQuote(bidm->getInfo(param.substr(0,n), param.substr(n+1, param.length())));
            } else {
                s << CtrlComm::xmlQuote(bidm->getInfo(param));
            }
        }
        break;
    case I_NUMAUCTIONMANAGERINFOS:
    default:
        return string();
    }

    s << "</info>" << endl;
    
    return s.str();
}


string Auctioner::getAuctionManagerInfo(infoList_t *i)
{
    ostringstream s;
    infoListIter_t iter;
   
    s << "<AuctionManagerInfos>\n";

    for (iter = i->begin(); iter != i->end(); iter++) {
        s << getInfo(iter->type, iter->param);
    }

    s << "</AuctionManagerInfos>\n";

    return s.str();
}


void Auctioner::handleGetInfo(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
        log->dlog(ch,"processing event Get info" );
#endif

     // get info types from event
     try {
         infoList_t *i = ((GetInfoEvent *)e)->getInfos(); 
         // send meter info
         comm->sendMsg(getAuctionManagerInfo(i), ((GetInfoEvent *)e)->getReq(), fds, 0 /* do not html quote */ );
     } catch(Error &err) {
         comm->sendErrMsg(err.getError(), ((GetInfoEvent *)e)->getReq(), fds);
     }

#ifdef DEBUG
        log->dlog(ch,"Ending event Get info" );
#endif
}


void Auctioner::handleGetModInfo(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
        log->dlog(ch,"processing event Get modinfo" );
#endif

    // get module information from loaded module (proc mods only now)
    try {

        string s = proc->getModuleInfoXML(((GetModInfoEvent *)e)->getModName());
        // send module info
        comm->sendMsg(s, ((GetModInfoEvent *)e)->getReq(), fds, 0);

#ifdef DEBUG
        log->dlog(ch,"Ending event Get modinfo" );
#endif

    } catch(Error &err) {
        comm->sendErrMsg(err.getError(), ((GetModInfoEvent *)e)->getReq(), fds);
    }
}

void Auctioner::handleAddBiddingObjects(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
        log->dlog(ch,"processing event adding bidding Objects" );
#endif


	biddingObjectDB_t *new_bids = NULL;

    try {
		// support only XML rules from file
        new_bids = bidm->parseBiddingObjects(((AddBiddingObjectsEvent *)e)->getFileName());

#ifdef DEBUG
        log->dlog(ch,"Bidding Objects sucessfully parsed " );
#endif
             
        // TODO AM : test rule spec 
        //proc->checkRules(new_rules);

#ifdef DEBUG
        log->dlog(ch,"Bidding Objects sucessfully checked " );
#endif

        // no error so lets add the bidding objects and schedule for activation
        // and removal
        bidm->addBiddingObjects(new_bids, evnt.get());

        saveDelete(new_bids);

		/*
		 * TODO AM : verify if I have to do this code.
			above 'addBids' produces an BidActivation event.
			if rule addition shall be performed _immediately_
			(fds == NULL) then we need to execute this
			activation event _now_ and not wait for the
			EventScheduler to do this some time later.
	    */
		if (fds == NULL ) {
			Event *e = evnt->getNextEvent();
			handleEvent(e, NULL);
			saveDelete(e);
		}

#ifdef DEBUG
        log->dlog(ch,"Ending event adding bidding objects" );
#endif

	} catch (Error &e) {
        // error in rule(s)
        if (new_bids) {
			saveDelete(new_bids);
		}
        throw e;
    }
}

void Auctioner::handleAddAuctions(Event *e, fd_sets_t *fds)
{
	
#ifdef DEBUG
    log->dlog(ch,"processing event adding auctions" );
#endif
	
	auctionDB_t *new_auctions = NULL;
	try {

        // Search the domain in the template container
        auctionerTemplateListIter_t iter = auctionerTemplates.find(domainId);
        
        // support only XML rules from file
        new_auctions = aucm->parseAuctions(((AddAuctionsEvent *)e)->getFileName(), iter->second);

#ifdef DEBUG
		iter = auctionerTemplates.find(domainId);
        log->dlog(ch,"Auctions sucessfully parsed Nbr Template: %d", (iter->second)->get_num_templates() );
        std::list<int> lstTmpId = (iter->second)->get_template_list();
        std::list<int>::iterator itLst;
        for (itLst = lstTmpId.begin(); itLst != lstTmpId.end(); ++itLst){
			log->dlog(ch,"templ Id: %d", *itLst);
		}
#endif
             
        // no error so lets add the rules and schedule for activation
        // and removal
        aucm->addAuctions(new_auctions, evnt.get());

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

#ifdef DEBUG
        log->dlog(ch,"Ending adding auctions events " );
#endif

     } catch (Error &e) {
         // error in rule(s)
         if (new_auctions) {
			 saveDelete(new_auctions);
         }
         throw e;
     }
}

void Auctioner::handleAddBiddingObjectsAuction(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"processing event add bidding Objects to auction" );
#endif		  

	biddingObjectDB_t *bids = NULL;

	try
	{
		
		bids = ((InsertBiddingObjectsAuctionEvent *) e)->getBiddingObjects();
		
	    // Verifies that every auction is included in the container
	    biddingObjectDBIter_t iter;
	    for (iter = bids->begin(); iter != bids->end(); ++iter)
	    {	
			string aSet = (*iter)->getAuctionSet();
			string aName = (*iter)->getAuctionName();
			Auction * a = aucm->getAuction(aSet, aName);
		
			if ((a == NULL) || (a->getState() != AO_ACTIVE)){
				log->dlog( ch, "Auction %s.%s not found in auction manager container", aSet.c_str(), aName.c_str() );
				throw Error("Auction %s.%s not found in auction manager container", aSet.c_str(), aName.c_str());
			}	
		}

	    // Proceed to insert the bid to its corresponding auction
	    for (iter = bids->begin(); iter != bids->end(); ++iter)
	    {
			string aSet = (*iter)->getAuctionSet();
			string aName = (*iter)->getAuctionName();
			Auction * a = aucm->getAuction(aSet, aName);
			proc->addBiddingObjectAuctionProcess(a->getUId(), *iter);
		}
		
		
#ifdef DEBUG
	log->dlog(ch,"Ending event add bidding Object to auction" );
#endif		  

	}
	catch (Error &err) {
		log->dlog( ch, err.getError().c_str() );
	}

}

void Auctioner::handleActivateAuction(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"processing event activate auctions" );
#endif

	auctionDB_t *auctions = NULL;

	try
	{		
	
		// get the auction involved.
		auctions = ((ActivateAuctionsEvent *)e)->getAuctions();

		// Add the auctions to the process.
		auctionDBIter_t iter;
		for (iter = auctions->begin(); iter != auctions->end(); ++iter){
			proc->addAuctionProcess(*iter, evnt.get());
		}
		
		// change their state to active
		aucm->activateAuctions(auctions, evnt.get());

#ifdef DEBUG
		log->dlog(ch,"Ending event activate auctions" );
#endif

	}
	catch (Error &err) {
		log->dlog( ch, err.getError().c_str() );
	}

}

void Auctioner ::handleActivateBiddingObjects(Event *e, fd_sets_t *fds)
{
#ifdef DEBUG
	log->dlog(ch,"Starting event Handle activate bidding Objects" );
#endif

	biddingObjectDB_t *bids = NULL;

	try
	{    		
		
		bids = ((ActivateBiddingObjectsEvent *)e)->getBiddingObjects();
		
		evnt->addEvent( new InsertBiddingObjectsAuctionEvent(*bids));	  
		
		bidm->activateBiddingObjects(bids);
		
#ifdef DEBUG
		log->dlog(ch,"Ending event remove bidding Objects" );
#endif		
	}
	catch (Error &err) {
		if (bids) {
			saveDelete(bids);
        }
		log->dlog( ch, err.getError().c_str() );		
	}



#ifdef DEBUG
	log->dlog(ch,"ending event Handle activate bidding Objects" );
#endif

}

void Auctioner::handleRemoveBiddingObjects(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"processing event remove bidding Objects" );
#endif

	biddingObjectDB_t *bids = NULL;

	try
	{    		
		
		bids = ((RemoveBiddingObjectsEvent *)e)->getBiddingObjects();

	    // Proceed to delete the bid from its corresponding auction
	    biddingObjectDBIter_t iter;
	    for (iter = bids->begin(); iter != bids->end(); ++iter)
	    {
			string aSet = (*iter)->getAuctionSet();
			string aName = (*iter)->getAuctionName();
			try{
				Auction * a = aucm->getAuction(aSet, aName);
				proc->delBiddingObjectAuctionProcess(a->getUId(), *iter);
			} catch(Error &e){
				//Nothing to do
			}
		}
			  
		// now get rid of the expired bid
		bidm->delBiddingObjects(bids, evnt.get());
		
#ifdef DEBUG
		log->dlog(ch,"Ending event remove bidding Objects" );
#endif		
	}
	catch (Error &err) {
		if (bids) {
			saveDelete(bids);
        }
		log->dlog( ch, err.getError().c_str() );		
	}
}

void Auctioner::handleRemoveBiddingObjectsAuction(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"processing event remove bidding object from auction" );
#endif

	biddingObjectDB_t *bids = NULL;

	try{
				
		int index = ((RemoveBiddingObjectsAuctionEvent *) e)->getIndex();
		bids = ((RemoveBiddingObjectsAuctionEvent *) e)->getBiddingObjects();

		proc->delBiddingObjectsAuctionProcess(index, bids);

#ifdef DEBUG
		log->dlog(ch,"Ending event remove bidding object from auction" );
#endif		
	} catch(Error &err) {
		if (bids){
			saveDelete(bids);
		}
		log->dlog( ch, err.getError().c_str() );
	}  

}


void Auctioner::handleProcModeleTimer(Event *e, fd_sets_t *fds)
{
#ifdef DEBUG
    log->dlog(ch,"processing event proc module timer" );
#endif

        // TODO AM : implement.
        //proc->timeout(((ProcTimerEvent *)e)->getID(), ((ProcTimerEvent *)e)->getActID(),
        //              ((ProcTimerEvent *)e)->getTID());

#ifdef DEBUG
    log->dlog(ch,"Ending event proc module timer" );
#endif        
}

void Auctioner::handlePushExecution(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
    log->dlog(ch,"processing event push execution" );
#endif

	try {
        
        int index = ((PushExecutionEvent *)e)->getIndex();
        
		// Execute the algorithm
        proc->executeAuction(index, evnt.get());
              
       // TODO AM: Send allocations for agents.
              

#ifdef DEBUG
		log->dlog(ch,"ending event push execution" );
#endif

	}  catch (Error &err) {
		log->dlog( ch, err.getError().c_str() );		
	}
}


void Auctioner::handleCreateCheckSession(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
    log->dlog(ch,"processing event create check session" );
#endif
	
	ipap_message *message = NULL;
	auctionDB_t *auctions = NULL;
	auction::Session *s = NULL;
	
	try{
		
		ostringstream os;
		message = ((CreateCheckSessionEvent *)e)->getMessage();
		auctions = proc->getApplicableAuctions(message);
		
		// Verify that a session can be created with data provided
		string sessionId = ((CreateSessionEvent *)e)->getSessionId();
		auction::Session *s  = new Session(sessionId);

		string sAddress, sPort;
		
		sPort = conf->getValue("ControlPort", "CONTROL");
		int port = atoi(sPort.c_str());

		string _uIPV6, sAddressIPV4, sAddressIPV6;
		 
		_uIPV6 = conf->getValue("UseIPv6", "CONTROL");
		int _useIPV6 = ParserFcts::parseBool(_uIPV6);
		bool useIPV6;

		if ( _useIPV6 == 1){ 
			useIPV6 = true;
			sAddressIPV6 = conf->getValue("LocalAddr-V6", "CONTROL");
		}
		else{
		 	useIPV6 = false; 
		 	sAddressIPV4 = conf->getValue("LocalAddr-V4", "CONTROL");
		}

		//! Set sender address, which is my own address.
		if (useIPV6){
			s->setSenderAddress(sAddressIPV6);
		} else {
			s->setSenderAddress(sAddressIPV4);
		}		
			
		//! Set sender port, which is my own auctioning port
		s->setSenderPort(port);

		map<ipap_field_key,string> dataSession = proc->getSessionInformation(message);
		if ( dataSession.size() == (proc->getSetField(AUM_SESSION_FIELD_SET_NAME)).size() )
		{
			map<ipap_field_key,string>::iterator dataSessionIter;
			
			dataSessionIter = dataSession.find(ipap_field_key(0,IPAP_FT_IPVERSION));
			string sIpVersion = dataSessionIter->second;
			string saddress;
			int ipVersion = atoi(sIpVersion.c_str());
			if (ipVersion == 4){
				dataSessionIter = dataSession.find(ipap_field_key(0,IPAP_FT_SOURCEIPV4ADDRESS));
				saddress = dataSessionIter->second;
			} else { // We assume ipv6
				dataSessionIter = dataSession.find(ipap_field_key(0,IPAP_FT_SOURCEIPV6ADDRESS));
				saddress = dataSessionIter->second;
			}	
				
			//! Set receiver address, which is my the agent requesting the session
			s->setReceiverAddress(saddress);

			//! Set source address, which is my the agent requesting the session
			s->setSourceAddress(saddress);
			
			dataSessionIter = dataSession.find(ipap_field_key(0,IPAP_FT_SOURCEAUCTIONPORT));
			string sPort = dataSessionIter->second;
			
			//! Set receiver port, which is the agent port
			s->setReceiverPort(atoi(sPort.c_str()));
			
			os << "<CreateCheckSession>\n";
			os << "<NbrAuctions>";
			os << auctions->size(); 
			os << "</NbrAuctions>\n";
			os << "</CreateCheckSession>";

		} else {
			os << "<CreateCheckSession>\n";
			os << "<NbrAuctions>";
			os << 0; // We can not create the session with data provided.
			os << "</NbrAuctions>\n";
			os << "</CreateCheckSession>";
		}
		
		saveDelete(s);
		saveDelete(auctions);
		
		comm->sendMsg(os.str().c_str(), ((CreateCheckSessionEvent *)e)->getReq(), fds);

#ifdef DEBUG
		log->dlog(ch,"Ending event create check session" );
#endif

	
	}  catch (Error &err) {
		if (auctions){
			saveDelete(auctions);
		}
		if (s){
			saveDelete(s);
		}
		log->dlog( ch, err.getError().c_str() );	
		comm->sendErrMsg(err.getError(), ((CreateCheckSessionEvent *)e)->getReq(), fds); 	
	}
}



void Auctioner::handleCreateSession(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
    log->dlog(ch,"processing event create session" );
#endif
	
	ipap_message *message = NULL;
	ipap_message *message_return = NULL;
	auctionDB_t *auctions = NULL;
	auction::Session *s = NULL;
	
	try{
		
		message = ((CreateSessionEvent *)e)->getMessage();
		auctions = proc->getApplicableAuctions(message);
		
#ifdef DEBUG
		log->dlog(ch,"# returned auctions: %d", auctions->size() );
#endif		

		// Read Local Address and port to send 
		string sAddress, sPort;
		
		sPort = conf->getValue("ControlPort", "CONTROL");
		int port = atoi(sPort.c_str());
		
		string _uIPV6, sAddressIPV4, sAddressIPV6;
		 
		_uIPV6 = conf->getValue("UseIPv6", "CONTROL");
		int _useIPV6 = ParserFcts::parseBool(_uIPV6);
		bool useIPV6;
		
		if ( _useIPV6 == 1){ 
			useIPV6 = true;
			sAddressIPV6 = conf->getValue("LocalAddr-V6", "CONTROL");
		}
		else{
		 	useIPV6 = false; 
		 	sAddressIPV4 = conf->getValue("LocalAddr-V4", "CONTROL");
		}

#ifdef DEBUG
		log->dlog(ch,"sAddressIPV6:%s", sAddressIPV6.c_str() );
#endif

        // Search the domain in the template container
        auctionerTemplateListIter_t iter = auctionerTemplates.find(domainId);

		message_return = aucm->get_ipap_message(auctions, iter->second, useIPV6, 
									sAddressIPV4, sAddressIPV6, port);

#ifdef DEBUG
		log->dlog(ch,"after building the message" );
#endif			
									
		anslp::msg::anslp_ipap_xml_message mess;
		anslp::msg::anslp_ipap_message anlp_mess(*message_return);
		
		saveDelete(message_return);
		string xmlMessage = mess.get_message(anlp_mess);
		
		// Only create the session, if the number of auctions is greater than zero.
		if (auctions->size() > 0){ 			
			string sessionId = ((CreateSessionEvent *)e)->getSessionId();
			s = new auction::Session(sessionId);

			// Bring the id of every auction in the auctionDB.
			auctionSet_t setAuc; 
			aucm->getIds(auctions, setAuc);

			// Add a new reference to the auction (there is another session reference it).
			aucm->incrementReferences(setAuc, sessionId);

			//! Set sender address, which is my own address.
			if (useIPV6){
				s->setSenderAddress(sAddressIPV6);
			} else {
				s->setSenderAddress(sAddressIPV4);
			}		
			
			//! Set sender port, which is my own auctioning port
			s->setSenderPort(port);

			map<ipap_field_key,string> dataSession = proc->getSessionInformation(message);
			if ( dataSession.size() == (proc->getSetField(AUM_SESSION_FIELD_SET_NAME)).size() )
			{
				map<ipap_field_key,string>::iterator dataSessionIter;
				
				dataSessionIter = dataSession.find(ipap_field_key(0,IPAP_FT_IPVERSION));
				string sIpVersion = dataSessionIter->second;
				string saddress;
				int ipVersion = atoi(sIpVersion.c_str());
				if (ipVersion == 4){
					dataSessionIter = dataSession.find(ipap_field_key(0,IPAP_FT_SOURCEIPV4ADDRESS));
					saddress = dataSessionIter->second;
				} else { // We assume ipv6
					dataSessionIter = dataSession.find(ipap_field_key(0,IPAP_FT_SOURCEIPV6ADDRESS));
					saddress = dataSessionIter->second;
				}	
					
				//! Set receiver address, which is my the agent requesting the session
				s->setReceiverAddress(saddress);

				//! Set source address, which is my the agent requesting the session
				s->setSourceAddress(saddress);
			
				dataSessionIter = dataSession.find(ipap_field_key(0,IPAP_FT_SOURCEAUCTIONPORT));
				string sPort = dataSessionIter->second;
				//! Set receiver port, which is the agent port
				s->setReceiverPort(atoi(sPort.c_str()));
				
			
			} else {
				throw("session information was not provided");
			}
			
			sesm->addSession(s); 
		}

		saveDelete(auctions);

		comm->sendMsg(xmlMessage.c_str(), ((CreateSessionEvent *)e)->getReq(), fds);

#ifdef DEBUG
		log->dlog(ch,"Ending event create session" );
#endif
		
	
	}  catch (Error &err) {
		if (message_return){
			saveDelete(message_return);
		}
		if (auctions){
			saveDelete(auctions);
		}
		if (s){
			saveDelete(s);
		}
		log->dlog( ch, err.getError().c_str() );	
		comm->sendErrMsg(err.getError(), ((CreateSessionEvent *)e)->getReq(), fds); 	
	}
}

void Auctioner::handleAuctioningInteraction(Event *e, fd_sets_t *fds)
{

	ipap_message *message = NULL;
	biddingObjectDB_t *bids = NULL;
	auction::Session *s = NULL;
	
	try {

		string sessionId = ((AuctionInteractionEvent *)e)->getSessionId();
		
		// Search for the session that is involved.
		s = sesm->getSession(sessionId);
		if (s == NULL)
			throw Error("Session %s not found", sessionId.c_str());

		message = ((AuctionInteractionEvent *)e)->getMessage();
		// get the msgSeqNbr from the message
		uint32_t seqNbr = message->get_seqno();
		
		// Bring the list of local templates
		auctionerTemplateListIter_t templIter = auctionerTemplates.find(domainId);
		if (templIter != auctionerTemplates.end()){
		
			bids = bidm->parseMessage(message,templIter->second);
			
			// Insert the session as part of the elements of bidding object
			biddingObjectDBIter_t bidIter;
			for (bidIter = bids->begin(); bidIter != bids->end(); ++bidIter){
				(*bidIter)->setSession(sessionId);
			}
			
			// Add the bidding objects to the bidding object manager.
			bidm->addBiddingObjects(bids, evnt.get());  
			
			// Build the response for the originator agent.
			ipap_message resp = ipap_message(domainId, IPAP_VERSION, true);
			resp.set_seqno(s->getNextMessageId());
			resp.set_ackseqno(seqNbr+1);
			resp.output();
			
			anslp::msg::anslp_ipap_xml_message mess;
			anslp::msg::anslp_ipap_message anlp_mess(resp);
			string xmlMessage = mess.get_message(anlp_mess);
			
			// Send the response message.
			comm->sendMsg(xmlMessage.c_str(), ((AuctionInteractionEvent *)e)->getReq(), fds);
		} else {
		  string error = "templates not initialized in the auctioneer"; 
		  comm->sendErrMsg(error, ((AuctionInteractionEvent *)e)->getReq(), fds); 			
		}
	} catch (Error &err){
		if (message){
			saveDelete(message);
        }
		log->dlog( ch, err.getError().c_str() );	
		comm->sendErrMsg(err.getError(), ((AuctionInteractionEvent *)e)->getReq(), fds); 			
	}

}

void Auctioner::handleAddGeneratedBiddingObjects(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
    log->dlog(ch,"processing event add generated bidding objects" );
#endif

	biddingObjectDB_t *new_bids = NULL;
	int index = 0;

   try {

       new_bids = ((AddGeneratedBiddingObjectsEvent *)e)->getBiddingObjects();
	   index = ((AddGeneratedBiddingObjectsEvent *)e)->getIndex();
	   
       // Add the new bidding object in the biddingObject manager
       bidm->addBiddingObjects(new_bids, evnt.get());

	   evnt.get()->addEvent(new TransmitBiddingObjectsEvent(index, *new_bids));

#ifdef DEBUG
       log->dlog(ch,"BiddingObjects sucessfully added " );
#endif


    } catch (Error &e) {
       throw e;
    }	
	
}

void 
Auctioner::handleTransmitBiddingObjects(Event *e, fd_sets_t *fds)
{
	ipap_message *mes = NULL;

	try{
		biddingObjectDB_t *new_bids = ((TransmitBiddingObjectsEvent *)e)->getBiddingObjects();
		
		// get the request index 
		int index = ((TransmitBiddingObjectsEvent *)e)->getIndex();

		// Bring the list of local templates
		auctionerTemplateListIter_t templIter = auctionerTemplates.find(domainId);
		if (templIter != auctionerTemplates.end()){
			
			biddingObjectDBIter_t iter;
			for (iter = new_bids->begin(); iter != new_bids->end(); ++iter)
			{
				// We find the auction for the bid
				BiddingObject *biddingObject = *iter;
			
				// Search for the corresponding session for this connection
				string sessionId = biddingObject->getSession();
				Session *session = sesm->getSession(sessionId);
			
				Auction *a = aucm->getAuction(biddingObject->getAuctionSet(), 
												biddingObject->getAuctionName());
				
				uint32_t mid = session->getNextMessageId();
				
				mes = bidm->get_ipap_message(biddingObject, a, templIter->second);
				mes->set_seqno(mid);
				mes->set_ackseqno(0);
				
				// Save the message within the pending messages.
				session->addPendingMessage(*mes);
				
				// Finally send the message through the anslp client application.
				anslpc->tg_bidding( new anslp::session_id(sessionId), 
									session->getReceiverAddress(), 
									session->getSenderAddress(), 
									session->getReceiverPort(), 
									session->getSenderPort(),
									session->getProtocol(), *mes );

				saveDelete(mes);
			}
		} else {
		  if (mes){
			saveDelete(mes);
		  }	
		  string error = "templates not initialized in the auctioneer"; 
		  throw Error(error);
		}
	} catch (Error &e){
		if (mes){
			saveDelete(mes);
		 }	
		throw e;
	}
}



/* -------------------- handleEvent -------------------- */

void Auctioner::handleEvent(Event *e, fd_sets_t *fds)
{
   
#ifdef DEBUG
        log->dlog(ch,"Start handleEvent" );
#endif   
   
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
    
    case GET_MODINFO:
		handleGetModInfo(e,fds);
		break;
    
    case ADD_BIDDING_OBJECTS:
		handleAddBiddingObjects(e,fds);      
		break;

    case ADD_AUCTIONS:
		handleAddAuctions(e,fds);
		break;

	case ADD_BIDDING_OBJECTS_AUCTION:
		handleAddBiddingObjectsAuction(e,fds);
		break;
		
	case ACTIVATE_BIDDING_OBJECTS:
		handleActivateBiddingObjects(e,fds);
		break;

    case ACTIVATE_AUCTIONS:
		handleActivateAuction(e,fds);
		break;

    case REMOVE_BIDDING_OBJECTS:
		handleRemoveBiddingObjects(e,fds);
		break;

	case REMOVE_BIDDING_OBJECTS_AUCTION:
		handleRemoveBiddingObjectsAuction(e,fds);
		break;

    case PROC_MODULE_TIMER:
		handleProcModeleTimer(e,fds);
		break;

    case PUSH_EXECUTION:
		handlePushExecution(e,fds);
		break;
    
    case CREATE_SESSION:
		handleCreateSession(e,fds);
		break;

    case CREATE_CHECK_SESSION:
		handleCreateCheckSession(e,fds);
		break;

	case AUCTION_INTERACTION_CTRLCOMM:
		handleAuctioningInteraction(e,fds);
		break;
		
    default:
#ifdef DEBUG
        log->dlog(ch,"Unknown event %s", eventNames[e->getType()].c_str() );
#endif
        throw Error("unknown event");
    }
}


/* ----------------------- run ----------------------------- */

void Auctioner::run()
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
		int t = comm->getTimeout();
		if (t > 0) {
			evnt->addEvent(new CtrlCommTimerEvent(t, t * 1000));
		}
		
		
        // start threads (if threading is configured)
        proc->run();

#ifdef DEBUG
        log->dlog(ch,"------- Auction Manager is running -------");
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
                   comm->handleFDEvent(&retEvents, &rset, &wset, &fds);
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

		log->log(ch,"NetAum going down on Ctrl-C" );

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

void Auctioner::dump(ostream &os)
{
    /* FIXME to be implemented */
    os << "dump" << endl;
}


/* ------------------------- operator<< ------------------------- */

ostream& auction::operator<<(ostream &os, Auctioner &obj)
{
    obj.dump(os);
    return os;
}

/* ------------------------ signal handler ---------------------- */

void Auctioner::sigint_handler(int i)
{
    char c = 'S';

    write(s_sigpipe[1], &c,1);
}

void Auctioner::sigusr1_handler(int i)
{
    char c = 'D';
    
    write(s_sigpipe[1], &c,1);
}

void Auctioner::exit_fct(void)
{
    unlink(NETAUM_LOCK_FILE.c_str());
}

void Auctioner::sigalarm_handler(int i)
{
    g_timeout = 1;
}

/* -------------------- alreadyRunning -------------------- */

int Auctioner::alreadyRunning()
{
    FILE *file;
    char cmd[128];
    struct stat stats;
    int status, oldPid;

	cout << NETAUM_LOCK_FILE.c_str() << endl;

    // do we have a lock file ?
    if (stat(NETAUM_LOCK_FILE.c_str(), &stats ) == 0) { 
		
		
		
        // read process ID from lock file
        file = fopen(NETAUM_LOCK_FILE.c_str(), "rt" );
        if (file == NULL) {
            throw Error("cannot open old pidfile '%s' for reading: %s\n",
                        NETAUM_LOCK_FILE.c_str(), strerror(errno));
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
        unlink(NETAUM_LOCK_FILE.c_str());
    }
	
	cout << NETAUM_LOCK_FILE.c_str() << endl;
	
    // no lock file and no running meter process
    // write new lock file and continue
    file = fopen(NETAUM_LOCK_FILE.c_str(), "wt" );
    if (file == NULL) {
        throw Error("cannot open pidfile '%s' for writing: %s\n",
                    NETAUM_LOCK_FILE.c_str(), strerror(errno));
    }
    
    cout << NETAUM_LOCK_FILE.c_str() << endl;
    
    fprintf(file, "%d\n", getpid());
    fclose(file);

    return 0;
}


void Auctioner::activateAuctions(auctionDB_t *auctions, EventScheduler *e)
{

#ifdef DEBUG    
    log->dlog(ch, "Activate auctions");
#endif  

    auctionDBIter_t             iter;
    auctionIntervalsIndexIter_t iter2;
    auctionIntervalsIndex_t     intervals;    

    for (iter = auctions->begin(); iter != auctions->end(); iter++) {
        Auction *a = (*iter);
        log->dlog(ch, "activate auction with name = '%s'", a->getAuctionName().c_str());
		
		// Create the execution intervals
		interval_t inter = a->getInterval();
		action_t *action = a->getAction();
        procdef_t entry;
        entry.interval = inter;
        entry.procname = action->name;              
        intervals[entry].push_back(a);
    }

    // group by export interval
    for (iter2 = intervals.begin(); iter2 != intervals.end(); iter2++) {
        unsigned long i = iter2->first.interval.interval;
#ifdef DEBUG    
    log->dlog(ch, "Activate auctions - Execution interval: %lu", i );
#endif  
	
	// Change the state of the auctions to active.
	aucm->activateAuctions(auctions, e);
        
        //e->addEvent(new PushExecutionEvent(i, iter2->second, iter2->first.procname,
        //                                i * 1000, iter2->first.interval.align));
    }
    

}
