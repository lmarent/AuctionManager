/// ----------------------------------------*- mode: C++; -*--
/// @file AnslpClient.cpp
/// The interface with the a-nslp application
/// ----------------------------------------------------------
/// $Id: AnslpClient.cpp 4118 2015-09-23 14:12:00 amarentes $
/// $HeadURL: https://./src/AnslpClient.cpp $
// ===========================================================
//                      
// Copyright (C) 2014-2015, all rights reserved by
// - Universidad de los Andes
//
// More information and contact:
// https://www.uniandes.edu.co
//                      
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ===========================================================
#include <unistd.h>

#include "logfile.h"
#include "fqueue.h"
#include "gist_conf.h"
#include "AnslpClient.h"
#include <time.h>
#include <pthread.h>
#include "ni_session.h"
#include <sys/syscall.h>



using namespace protlib;
using namespace protlib::log;
using namespace ntlp;
using namespace anslp;
using namespace anslp::msg;
using namespace auction;

namespace ntlp {
// configuration class
gistconf gconf;
}

AnslpClient::AnslpClient(string config_filename, anslp::FastQueue *installQueue): 
starter(NULL), conf(NULL), anslpd(NULL)
{
	using namespace std;

    log = Logger::getInstance();
    ch = log->createChannel("AnslClient");

#ifdef DEBUG
    log->dlog(ch,"Starting AnslpClient");
#endif


	hostaddress source;
	
	try {

		conf = new anslp_config(); 	
		
		// create the global configuration parameter repository 
		conf->repository_init();

		// register all A-NSLP configuration parameters at the registry
		conf->setRepository();

		// register all GIST configuration parameters at the registry
		ntlp::gconf.setRepository();
		
		ntlp::gconf.var = 10;
		
		// read all config values from config file
		configfile cfgfile(configpar_repository::instance());
		conf->getparref<string>(anslpconf_conffilename) = config_filename;

#ifdef DEBUG
    log->dlog(ch,"anslp configuration object created %s", 
			(conf->getparref<string>(anslpconf_conffilename)).c_str());
#endif	

		cfgfile.load(conf->getparref<string>(anslpconf_conffilename));

#ifdef DEBUG
		log->dlog(ch,"Parameters registered \n %s", ntlp::gconf.to_string().c_str());
#endif	
		
	}
	catch(configParException& cfgerr)
	{
		log->elog(ch,"Error occurred while reading the configuration file %s", cfgerr.what());
	}

	/*
	 * Start the A-NSLP daemon thread. It will in turn start the other
	 * threads it requires.
	 */
	anslp_daemon_param param("anslp", *conf, installQueue);
	starter = new protlib::ThreadStarter<anslp_daemon, anslp_daemon_param>(3, param);
	
	// returns after all threads have been started
	starter->start_processing();

	anslpd = starter->get_thread_object();

	log->log(ch,"config file:%s", config_filename.c_str());

#ifdef DEBUG
    log->dlog(ch,"ending constructor AnslpClient");
#endif
	
}

AnslpClient::~AnslpClient()
{
	
#ifdef DEBUG
    log->dlog(ch,"Starting destructor AnslpClient");
#endif

	// shutdown mnslp thread
	starter->stop_processing();
	starter->wait_until_stopped();
	
	saveDelete(starter);
	
	saveDelete(conf);

#ifdef DEBUG
    log->dlog(ch,"Ending destructor AnslpClient");
#endif

}

void
AnslpClient::tg_create( const string sessionId, 
						const hostaddress &source_addr, 
						const hostaddress &destination_addr,
					    uint16_t source_port, uint16_t dest_port, 
						uint8_t protocol, uint32_t session_lifetime, 
						ipap_message &message )
{
#ifdef DEBUG
    log->dlog(ch,"Starting tg_create");
#endif
	
	anslp_ipap_message mess(message); 
		
    // Build the vector of objects to be configured.
    vector<msg::anslp_mspec_object *> mspec_objects;
    mspec_objects.push_back(mess.copy());

#ifdef DEBUG
    log->dlog(ch,"message pushed");
#endif

    // Create a new event for launching the configure event.
    event *e = new api_create_event(sessionId, source_addr, destination_addr, source_port, 
   				       dest_port, protocol, mspec_objects, 
				       session_lifetime, 
				       selection_auctioning_entities::sme_any, 
					   anslpd->getInstallQueue());

    anslp_event_msg *msg = new anslp_event_msg(session_id(), e);
        
//#ifdef DEBUG
    ostringstream o;
    o << "api event created message:" << msg->get_id();
    o << " session id:" << msg->get_session_id().to_string(); 
    log->log(ch,"%s", o.str().c_str());
//#endif
	
    bool queued = false;

#ifdef DEBUG
		log->dlog(ch,"nbr of messages in queue:%s - %lu", anslpd->get_fqueue()->get_name(), anslpd->get_fqueue()->size());
#endif    

    log->log(ch,"it is going to create the session - procid:%d - getthread_self:%lu tid:%lu", 
					getpid(), pthread_self(), syscall(SYS_gettid));
    
    queued = anslpd->get_fqueue()->enqueue(msg);
    
    if ( ! queued ){
		
		throw Error("Error creating the api event");		
			
	} 
}

void 
AnslpClient::tg_teardown(anslp::session_id *sid) 
{

#ifdef DEBUG
    log->dlog(ch,"Starting tg_teardown ");
#endif        

	event *e = new api_teardown_event(sid);

	anslp_event_msg *msg = new anslp_event_msg(*sid, e);
		
	anslpd->get_fqueue()->enqueue(msg);
	
#ifdef DEBUG
    log->dlog(ch,"Ending tg_teardown ");
#endif        

}


void 
AnslpClient::tg_check(string sessionId, vector<msg::anslp_mspec_object *> mspec_objects)
{


//#ifdef DEBUG
    log->log(ch,"Starting tg_check sessionId:%s", sessionId.c_str());
//#endif        
	
	anslp::session_id *sid = new anslp::session_id(sessionId);
	
    // Create a new event for launching the configure event.
    anslp::event *e = new anslp::api_check_event(sid, mspec_objects, NULL);

    anslp_event_msg *msg = new anslp_event_msg(*sid, e);

//#ifdef DEBUG
    log->log(ch,"Message id: %lld sessionid:%s", msg->get_id(), 
							msg->get_session_id().to_string().c_str());
//#endif	
	
    anslpd->get_fqueue()->enqueue(msg);
	
        
//#ifdef DEBUG
    log->log(ch,"Ending tg_check ");
//#endif  

}


void 
AnslpClient::tg_install(string sessionId, 
						vector<msg::anslp_mspec_object *> mspec_objects)
{


//#ifdef DEBUG
    log->log(ch,"Starting tg_install sessionId:%s", sessionId.c_str());
//#endif        
	
	anslp::session_id *sid = new anslp::session_id(sessionId);
	
    // Create a new event for launching the configure event.
    anslp::event *e = new anslp::api_install_event(sid, mspec_objects, NULL);

    anslp_event_msg *msg = new anslp_event_msg(*sid, e);

//#ifdef DEBUG
    log->log(ch,"Message id: %lld sessionid:%s", msg->get_id(), 
							msg->get_session_id().to_string().c_str());
//#endif	
	
    anslpd->get_fqueue()->enqueue(msg);
	
        
//#ifdef DEBUG
    log->log(ch,"Ending tg_install ");
//#endif  

}

void 
AnslpClient::tg_bidding(anslp::session_id *sid, 
						const protlib::hostaddress &source_addr, 
						const protlib::hostaddress &destination_addr,
					    uint16_t source_port, uint16_t dest_port, 
						uint8_t protocol, ipap_message &message)
{


#ifdef DEBUG
    log->dlog(ch,"Starting tg_bidding ");
#endif        

    // Build an ipap_message for a create session, which only has 
    // auction template options.
	// Build the request message 

	anslp_ipap_message mess(message);    
            	    
    // Build the vector of objects to be configured.
    vector<msg::anslp_mspec_object *> mspec_objects;
    mspec_objects.push_back(mess.copy());

    // Create a new event for launching the configure event.
    event *e = new api_bidding_event(sid, source_addr, destination_addr, source_port, 
   				       dest_port, protocol, mspec_objects, NULL);

    anslp_event_msg *msg = new anslp_event_msg(*sid, e);

#ifdef DEBUG
    log->dlog(ch,"Message id: %lld", msg->get_id());
#endif	
	
    anslpd->get_fqueue()->enqueue(msg);
	
        
#ifdef DEBUG
    log->dlog(ch,"Ending tg_bidding ");
#endif  

}

anslp_event_msg *
AnslpClient::delayed_tg_bidding(anslp::session_id *sid, 
								const protlib::hostaddress &source_addr, 
								string destination_addr,
								uint16_t source_port, uint16_t dest_port, 
								uint8_t protocol, ipap_message &message)
{

#ifdef DEBUG
    log->dlog(ch,"Starting delayed_tg_bidding ");
#endif        

	protlib::hostaddress dest_addr;
	dest_addr.set_ip(destination_addr);

    // Build an ipap_message for a create session, which only has 
    // auction template options.
	// Build the request message 

	anslp_ipap_message mess(message);    
            	    
    // Build the vector of objects to be configured.
    vector<msg::anslp_mspec_object *> mspec_objects;
    mspec_objects.push_back(mess.copy());

    // Create a new event for launching the configure event.
    event *e = new api_bidding_event(sid, source_addr, dest_addr, source_port, 
   				       dest_port, protocol, mspec_objects, NULL);

    anslp_event_msg *msg = new anslp_event_msg(*sid, e);
	        
#ifdef DEBUG
    log->dlog(ch,"Ending delayed_tg_bidding - Message id: %lld", msg->get_id());
#endif  

	return msg;
}

void 
AnslpClient::tg_bidding(std::vector<anslp::anslp_event_msg *> *events)
{
	std::vector<anslp::anslp_event_msg *>::iterator it;
	
	for (it = events->begin(); it != events->end(); it++)
	{
		anslpd->get_fqueue()->enqueue(*it);
	}
}

void 
AnslpClient::tg_bidding(anslp::session_id *sid, 
						const protlib::hostaddress &source_addr, 
						string destination_addr,
					    uint16_t source_port, uint16_t dest_port, 
						uint8_t protocol, ipap_message &message)
{

   protlib::hostaddress dest_addr;
   dest_addr.set_ip(destination_addr);
 
   tg_bidding(sid, source_addr, dest_addr, source_port, dest_port, protocol, message);

}						


void 
AnslpClient::tg_remove(string sessionId, vector<msg::anslp_mspec_object *> mspec_objects)
{


//#ifdef DEBUG
    log->log(ch,"Starting tg_remove sessionId:%s", sessionId.c_str());
//#endif        
	
	anslp::session_id *sid = new anslp::session_id(sessionId);
	
    // Create a new event for launching the configure event.
    anslp::event *e = new anslp::api_remove_event(sid, mspec_objects, NULL);

    anslp_event_msg *msg = new anslp_event_msg(*sid, e);

//#ifdef DEBUG
    log->log(ch,"Message id: %lld sessionid:%s", msg->get_id(), 
							msg->get_session_id().to_string().c_str());
//#endif	
	
    anslpd->get_fqueue()->enqueue(msg);
	
        
//#ifdef DEBUG
    log->log(ch,"Ending tg_install ");
//#endif  

}



string AnslpClient::getLocalAddress(void)
{
	list<hostaddress>::iterator iter;
	list<hostaddress> addresses4 = ntlp::gconf.getpar< list<hostaddress> >(gistconf_localaddrv4);
	
	for (iter = addresses4.begin(); iter != addresses4.end(); ++iter){
		string address(iter->get_ip_str());
		if (!(address.empty()))
			return address;
	}

	list<hostaddress> addresses6 = ntlp::gconf.getpar< list<hostaddress> >(gistconf_localaddrv6);
	for (iter = addresses6.begin(); iter != addresses6.end(); ++iter){
		string address(iter->get_ip_str());
		if (!(address.empty()))
			return address;
	}
	
	return "";
}

uint32_t AnslpClient::getInitiatorLifetime(void)
{
	if (conf){
		uint32_t lifetime = conf->get_ni_session_lifetime();
		return lifetime;
	}
	else{
		throw Error("AnslpClient not configured");
	}
	
	return 0;
}
