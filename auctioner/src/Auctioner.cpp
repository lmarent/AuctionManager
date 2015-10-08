
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
#include "AnslpClient.h"
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

        // set the domain id
        string _domainId = conf->getValue("Domain", "MAIN");
		domainId = ParserFcts::parseInt( _domainId );

#ifdef DEBUG
        log->log(ch,"domainId used is: '%d'", domainId);
#endif

        // startup other core classes
        auto_ptr<AuctionTimer> _auct(AuctionTimer::getInstance());
        auct = _auct;
        
        auto_ptr<BidManager> _bidm(new BidManager(conf->getValue("FieldDefFile", "MAIN"),
                                                    conf->getValue("FieldConstFile", "MAIN")));
        bidm = _bidm;
        
        auto_ptr<AuctionManager> _aucm(new AuctionManager(conf->getValue("FieldDefFile", "MAIN"),
														   conf->getValue("FieldConstFile", "MAIN")));
        aucm = _aucm;

        auto_ptr<SessionManager> _sesm(new SessionManager());
        sesm = _sesm;
        
        auto_ptr<EventSchedulerAuctioner> _evnt(new EventSchedulerAuctioner());
        evnt = _evnt;

		auto_ptr<MAPIAuctionParser> _mpap(new MAPIAuctionParser());
		mpap = _mpap;
	
        // Startup Processing Components

#ifdef ENABLE_THREADS

        auto_ptr<AUMProcessor> _proc(new AUMProcessor(conf.get(),
							    conf->isTrue("Thread",
									 "AUM_PROCESSOR")));
        pprocThread = conf->isTrue("Thread", "AUM_PROCESSOR");
#else
        
        auto_ptr<AUMProcessor> _proc(new AUMProcessor(conf.get(), 
									 conf->getValue("FieldDefFile", "MAIN"), 
									 0));
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

		enableCtrl = conf->isTrue("Enable", "CONTROL");

		if (enableCtrl) {
			// ctrlcomm can never be a separate thread
			auto_ptr<CtrlComm> _comm(new CtrlComm(conf.get(), 0));
			comm = _comm;
			comm->mergeFDs(&fdList);
		}

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
        s << bidm->getNumBids();
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

void Auctioner::handleAddBids(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
        log->dlog(ch,"processing event adding bids" );
#endif


	bidDB_t *new_bids = NULL;

    try {
		// support only XML rules from file
        new_bids = bidm->parseBids(((AddBidsEvent *)e)->getFileName());

#ifdef DEBUG
        log->dlog(ch,"Bids sucessfully parsed " );
#endif
             
        // TODO AM : test rule spec 
        //proc->checkRules(new_rules);

#ifdef DEBUG
        log->dlog(ch,"Bids sucessfully checked " );
#endif

        // no error so lets add the rules and schedule for activation
        // and removal
        bidm->addBids(new_bids, evnt.get());

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
        log->dlog(ch,"Ending event adding bids" );
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

        // support only XML rules from file
        new_auctions = aucm->parseAuctions(((AddAuctionsEvent *)e)->getFileName());

#ifdef DEBUG
        log->dlog(ch,"Auctions sucessfully parsed " );
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

void Auctioner::handleAddBidsCntrlComm(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
    log->dlog(ch,"processing event add bids by controlcomm" );
#endif

	bidDB_t *new_bids = NULL;
	try {
              
        new_bids = bidm->parseBidsBuffer(
        ((AddBidsCtrlEvent *)e)->getBuf(),
        ((AddBidsCtrlEvent *)e)->getLen());
	  
        // no error so let's add the rules and 
        // schedule for activation and removal
        bidm->addBids(new_bids, evnt.get());
        comm->sendMsg("bid(s) added", ((AddBidsCtrlEvent *)e)->getReq(), fds);
        saveDelete(new_bids);

#ifdef DEBUG
    log->dlog(ch,"Ending event add bids by controlcomm" );
#endif

    } catch (Error &err) {
        // error in bid(s)
        if (new_bids) {
             saveDelete(new_bids);
        }
        comm->sendErrMsg(err.getError(), ((AddBidsCtrlEvent *)e)->getReq(), fds); 
    }
}

void Auctioner::handleAddBidsAuction(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"processing event add bid auction" );
#endif		  

	Bid *b = NULL;

	try{
		
		b = ((InsertBidAuctionEvent *) e)->getBid();

		string auctionSet = ((InsertBidAuctionEvent *) e)->getAuctionSet();          
		string auctionName = ((InsertBidAuctionEvent *) e)->getAuctionName();
                    
		proc->addBidAuction(auctionSet, auctionName, b);

#ifdef DEBUG
	log->dlog(ch,"Ending event add bid auction" );
#endif		  

	}
	catch (Error &err) {
		if (b){
			saveDelete(b);
		}
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
	
		auctions = ((ActivateAuctionsEvent *)e)->getAuctions();

		proc->addAuctions(auctions, evnt.get());
		// activate
		activateAuctions(auctions, evnt.get());

#ifdef DEBUG
		log->dlog(ch,"Ending event activate auctions" );
#endif

	}
	catch (Error &err) {
        if (auctions) {
             saveDelete(auctions);
        }
		log->dlog( ch, err.getError().c_str() );
	}

}

void Auctioner::handleRemoveBids(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"processing event remove bids" );
#endif

	bidDB_t *bids = NULL;

	try
	{    		
		
		bids = ((RemoveBidsEvent *)e)->getBids();
			  
		// now get rid of the expired bid
		proc->delBids(bids);
		bidm->delBids(bids, evnt.get());
		
#ifdef DEBUG
		log->dlog(ch,"Ending event remove bids" );
#endif		
	}
	catch (Error &err) {
		if (bids) {
			saveDelete(bids);
        }
		log->dlog( ch, err.getError().c_str() );		
	}
}

void Auctioner::handleRemoveBidsAuction(Event *e, fd_sets_t *fds)
{

#ifdef DEBUG
	log->dlog(ch,"processing event remove bid from auction" );
#endif

	Bid *b = NULL;

	try{
				
		b = ((RemoveBidAuctionEvent *) e)->getBid();
		string auctionSet = ((RemoveBidAuctionEvent *) e)->getAuctionSet();
		string auctionName = ((RemoveBidAuctionEvent *) e)->getAuctionName();
		proc->delBidAuction(auctionSet, auctionName, b);

#ifdef DEBUG
		log->dlog(ch,"Ending event remove bid from auction" );
#endif		
	} catch(Error &err) {
		if (b){
			saveDelete(b);
		}
		log->dlog( ch, err.getError().c_str() );
	}  

}

void Auctioner::handleRemoveBidsCntrlComm(Event *e, fd_sets_t *fds)
{
	try 
	{

#ifdef DEBUG
		log->dlog(ch,"processing event remove bids cntrlcomm" );
#endif

        string r = ((RemoveBidsCtrlEvent *)e)->getBid();
        int n = r.find(".");

        if (n > 0) {
			string sname = r.substr(0,n); 
			string rname = r.substr(n+1, r.length()-n);
#ifdef DEBUG
			log->dlog(ch,"Deleting bid set=%s ruleId=%s", sname.c_str(), rname.c_str() );
#endif

            // delete 1 bid
            Bid *rptr = bidm->getBid(sname, rname);
            if (rptr == NULL) {
				throw Error("no such bid");
            }
                  
			bidAuctionListIter_t bidauct_iter;
			for (bidauct_iter = (rptr->getAuctions())->begin(); 
					bidauct_iter != (rptr->getAuctions())->end(); 
						++bidauct_iter ){
				proc->delBidAuction((bidauct_iter->second).auctionSet, 
									(bidauct_iter->second).auctionName, rptr);
			}
                  
            bidm->delBid(rptr, evnt.get());

        } else {

#ifdef DEBUG
			log->dlog(ch,"Deleting bid set=%s ", r.c_str() );
#endif				  
            // delete rule set
            bidIndex_t *bids = bidm->getBids(r);
            if (bids == NULL) {
				throw Error("no such bid set");
			}

            for (bidIndexIter_t i = bids->begin(); i != bids->end(); i++) {
				Bid *rptr = bidm->getBid(i->second);

				bidAuctionListIter_t bidauct_iter;
				for (bidauct_iter = (rptr->getAuctions())->begin(); 
					  bidauct_iter != (rptr->getAuctions())->end(); 
						++bidauct_iter ){
					proc->delBidAuction((bidauct_iter->second).auctionSet, 
									   (bidauct_iter->second).auctionName, rptr);
				}

                bidm->delBid(rptr, evnt.get());
            }
        }
        comm->sendMsg("bid(s) deleted", ((RemoveBidsCtrlEvent *)e)->getReq(), fds);

#ifdef DEBUG
		log->dlog(ch,"Ending event remove bids cntrlcomm" );
#endif

        
   } catch (Error &err) {
        comm->sendErrMsg(err.getError(), ((RemoveBidsCtrlEvent *)e)->getReq(), fds);
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

    auctionDB_t *auctions = NULL;

	try {
        auctions = ((PushExecutionEvent *)e)->getAuctions();

        // multiple auctions can process at the same time
        for (auctionDBIter_t iter = auctions->begin(); iter != auctions->end(); iter++) {
			
			// Execute the algorithm
            proc->executeAuction((*iter)->getUId(), (*iter)->getAuctionName());
              
            // TODO AM: Send allocations for agents.
              
        }

#ifdef DEBUG
		log->dlog(ch,"ending event push execution" );
#endif

	}  catch (Error &err) {
        if (auctions) {
             saveDelete(auctions);
        }
		log->dlog( ch, err.getError().c_str() );		
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

		string sessionId = ((CreateSessionEvent *)e)->getSessionId();
		s = new auction::Session(sessionId);
		sesm->addSession(s); 
		
		message = ((CreateSessionEvent *)e)->getMessage();
		auctions = proc->getApplicableAuctions(message);
		
#ifdef DEBUG
		log->dlog(ch,"# returned auctions: %d", auctions->size() );
#endif		

		message_return = mpap->get_ipap_message(proc->getFieldDefinitions(), auctions);
		anslp::msg::anslp_ipap_xml_message mess;
		anslp::msg::anslp_ipap_message anlp_mess(*message_return);
		
		saveDelete(message_return);
		string xmlMessage = mess.get_message(anlp_mess);
		
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
/* -------------------- handleEvent -------------------- */

void Auctioner::handleEvent(Event *e, fd_sets_t *fds)
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
    
    case GET_MODINFO:
		handleGetModInfo(e,fds);
		break;
    
    case ADD_BIDS:
		handleAddBids(e,fds);      
		break;

    case ADD_AUCTIONS:
		handleAddAuctions(e,fds);
		break;

    case ADD_BIDS_CTRLCOMM:
		handleAddBidsCntrlComm(e,fds);
		break; 	

	case ADD_BID_AUCTION:
		handleAddBidsAuction(e,fds);
		break;

    case ACTIVATE_AUCTIONS:
		handleActivateAuction(e,fds);
		break;

    case REMOVE_BIDS:
		handleRemoveBids(e,fds);
		break;

	case REMOVE_BID_AUCTION:
		handleRemoveBidsAuction(e,fds);
		break;

    case REMOVE_BIDS_CTRLCOMM:
		handleRemoveBidsCntrlComm(e,fds);
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
		if (enableCtrl) {
		  int t = comm->getTimeout();
		  if (t > 0) {
				evnt->addEvent(new CtrlCommTimerEvent(t, t * 1000));
		  }
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
        a->setState(AS_ACTIVE);
		
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
        
        //e->addEvent(new PushExecutionEvent(i, iter2->second, iter2->first.procname,
        //                                i * 1000, iter2->first.interval.align));
    }
    

}
