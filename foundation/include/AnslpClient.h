/// ----------------------------------------*- mode: C++; -*--
/// @file AnslpClient.h
/// The interface with the a-nslp application
/// ----------------------------------------------------------
/// $Id: AnslpClient.h 4118 2015-09-23 14:07:00 amarentes $
/// $HeadURL: https://./src/AnslpClient.h $
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

#ifndef _ANSLP_CLIENT_H_
#define _ANSLP_CLIENT_H_

#include <unistd.h>

#include "stdincpp.h"
#include "anslp_config.h"
#include "events.h"

#include "gist_conf.h"
#include "configfile.h"
#include "anslp_daemon.h"
#include "auction_rule.h"
#include "msg/anslp_ipap_message.h"


namespace auction
{

class AnslpClient
{

	private:
	
		
		Logger *log;
		int ch; //! logging channel number used by objects of this class
	
		//! config file name
		string config_filename;

		//! Pointer to the thread with the deamon for the anslp application
		protlib::ThreadStarter<anslp::anslp_daemon, anslp::anslp_daemon_param> *starter;

		//! Pointer to anslp application configuration object.
		anslp::anslp_config			*conf;

		//! Pointer to the anslp deamon object .
		anslp::anslp_daemon *anslpd; 

	public:
		
		AnslpClient(string config_filename, anslp::FastQueue *installQueue=NULL);
		
		~AnslpClient();
		
		string getLocalAddress(void);
		
		inline anslp::anslp_daemon * getAnslpDeamon(){ return anslpd; }
		
		uint32_t getInitiatorLifetime(void);
		 
		void tg_create( const string sessionId,  const protlib::hostaddress &source_addr, 
			    		const protlib::hostaddress &destination_addr,
						uint16_t source_port, uint16_t dest_port, 
						uint8_t protocol, uint32_t session_lifetime,
						ipap_message &message );

		void tg_teardown(anslp::session_id *sid);

		void tg_install(string sessionId, 
						vector<anslp::msg::anslp_mspec_object *> mspec_objects);

		void tg_bidding(anslp::session_id *sid, 
						const protlib::hostaddress &source_addr, 
						const protlib::hostaddress &destination_addr,
					    uint16_t source_port, uint16_t dest_port, 
						uint8_t protocol, ipap_message &message);

		anslp::anslp_event_msg *
		delayed_tg_bidding(anslp::session_id *sid, 
						   const protlib::hostaddress &source_addr, 
						   string destination_addr,
						   uint16_t source_port, uint16_t dest_port, 
						   uint8_t protocol, ipap_message &message);

		//! Send delayed events for the anslp queue for processing.
		void tg_bidding(std::vector<anslp::anslp_event_msg *> *events);

		void tg_bidding(anslp::session_id *sid, 
						const protlib::hostaddress &source_addr, 
						string destination_addr,
						uint16_t source_port, uint16_t dest_port, 
						uint8_t protocol, ipap_message &message);


};

}

#endif // _ANSLP_CLIENT_H
