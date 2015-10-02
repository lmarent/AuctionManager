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
		auto_ptr<anslp::anslp_config>			conf;

		anslp::session_id sid;
	  
		//! sender host address.
		protlib::hostaddress sender_addr;
		
		//! Receiver host address.
		protlib::hostaddress receiver_addr;

		//! Source address.
		protlib::hostaddress source;
		
		//! Sender port
		uint16_t sender_port;
		
		//! Receiver port
		uint16_t receiver_port;
		
		//! Protocol
		uint8_t protocol;
		
		//! lifetime by default.
		uint32_t lifetime;


	public:
		
		AnslpClient(string config_filename, 
					const protlib::hostaddress &source_addr, 
					const protlib::hostaddress &destination_addr,
					uint16_t source_port, uint16_t dest_port, 
					uint8_t protocol, uint32_t session_lifetime);
		
		~AnslpClient();

		//void run();

		//vector<ipap_message *> 
		void tg_create( const protlib::hostaddress &source_addr, 
					    const protlib::hostaddress &destination_addr,
					   uint16_t source_port, uint16_t dest_port, 
					    uint8_t protocol, uint32_t session_lifetime );

		void tg_teardown(anslp::session_id id);

		void tg_bidding(const protlib::hostaddress &source_addr, 
						const protlib::hostaddress &destination_addr,
					   uint16_t source_port, uint16_t dest_port, 
						uint8_t protocol, uint32_t session_lifetime);
		
		//! Set sender address 
		void set_sender_address(protlib::hostaddress _sender_address);

		//! Set receiver address 
		void set_receiver_address(protlib::hostaddress _receiver_address);

		//! Set source address 
		void set_source_address(protlib::hostaddress _source_address);

		//! Set sender port
		void set_sender_port(uint16_t _sender_port);

		//! Set receiver port
		void set_receiver_port(uint16_t _receiver_port);

		//! Set protocol
		void set_protocol(uint8_t protocol);

		//! Set lifetime
		void set_lifetime(uint32_t lifetime);

};

};
