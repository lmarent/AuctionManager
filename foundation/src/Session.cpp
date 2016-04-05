/// ----------------------------------------*- mode: C++; -*--
/// @file Session.cpp
/// The Session class.
/// ----------------------------------------------------------
/// $Id: Session.cpp 2558 2015-10-02 6:16:00 $
/// $HeadURL: https://./src/Session.cpp $
// ===========================================================
//                      
// Copyright (C) 2012-2014, all rights reserved by
// - System and Computing Engineering, Universidad de los Andes
//
// More information and contact:
// https://www.uniandes.edu.co/
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

#include "Session.h"

using namespace auction;


/**
 * Constructor.
 *
 */
Session::Session(string _sessionId):
	state(SS_NEW), sessionId(_sessionId), mId()
{
	
}

/**
 * Destructor.
 *
 */
Session::~Session()
{
	pendingMessages.clear();
}

std::ostream& operator<<(std::ostream &out, const Session &obj) 
{
	using namespace std;

	out << obj.getSessionId();

	return out;
}

/* ------------------------ setSenderAddress -----------------------*/
void 
Session::setSenderAddress(string _sender_address)
{
	
	sender_addr.set_ip(_sender_address);
}

/* ------------------------ getSenderAddress -----------------------*/
protlib::hostaddress 
Session::getSenderAddress()
{
	return sender_addr;
}

/* ----------------------- setReceiverAddress ----------------------*/
void
Session::setReceiverAddress(string _receiver_address)
{
	receiver_addr.set_ip(_receiver_address);
}

/* ----------------------- getReceiverAddress ----------------------*/
protlib::hostaddress
Session::getReceiverAddress()
{
	
	return receiver_addr;
}

/* ------------------------ setSourceAddress -----------------------*/
void
Session::setSourceAddress(string _source_address)
{
	source.set_ip(_source_address);
}

/* ------------------------ getSourceAddress -----------------------*/
protlib::hostaddress
Session::getSourceAddress()
{
	return source;
}

/* ------------------------- setSenderPort -------------------------*/
void
Session::setSenderPort(uint16_t _sender_port)
{
	sender_port = _sender_port;
}

/* ------------------------- getSenderPort -------------------------*/
uint16_t
Session::getSenderPort()
{
	return sender_port;
}


/* ------------------------ setReceiverPort ------------------------*/
void
Session::setReceiverPort(uint16_t _receiver_port)
{
	receiver_port = _receiver_port;
}

/* ------------------------ getReceiverPort ------------------------*/
uint16_t
Session::getReceiverPort()
{
	return receiver_port;
}


/* -------------------------- setProtocol ---------------------------*/
void
Session::setProtocol(uint8_t _protocol)
{
	protocol = _protocol;
}

/* -------------------------- getProtocol ---------------------------*/
uint8_t
Session::getProtocol()
{
	return protocol;
}

/* ------------------------ setLifetime -----------------------*/
void
Session::setLifetime(uint32_t _lifetime)
{
	lifetime = _lifetime;
}

/* ------------------------ getLifetime -----------------------*/
uint32_t
Session::getLifetime()
{
	return lifetime;
}

/* ------------------------ getMessageId -----------------------*/

uint32_t 
Session::getNextMessageId()
{
	return mId.newId();
}

void 
Session::setAnlspSession(string _sid)
{
	sid = _sid;
}

string Session::getInfo()
{

	ostringstream os;
	
	os << "id:" << getUId() << " state:" << getState()
	   << " sessionid:" << getSessionId() << endl;
	   
	os << " senderAddress:" << getSenderAddress().get_ip_str() 
	   << " senderPort" << getSenderPort() << "|";
	   
	os << " receiverAddress:" << getReceiverAddress().get_ip_str() 
	   << " receiverPort" << getReceiverPort() << endl << "|";
	   
	os << " sourceAddress:" << getSourceAddress().get_ip_str() << endl;
	
	os << " protocol:" << getProtocol() << " lifetime:" << getLifetime() << endl;
	
	os << " anslp-session:" <<  getAnlspSession() << endl;
	
	os << "last Message Nbr:" << mId << endl;
	
	return os.str();
}

//! Confirm the response of the message with id mid.
void Session::confirmMessage(uint32_t mid)
{
	pendingMessageListIter_t iter = pendingMessages.find(mid);
	if (iter != pendingMessages.end()){
		pendingMessages.erase(iter);
	}	
}
	
//! add to the list of pensing message a new entry.
void Session::addPendingMessage(ipap_message mes)
{
	uint32_t mid = mes.get_seqno();
	pendingMessageListIter_t iter = pendingMessages.find(mid);
	if (iter == pendingMessages.end()){
		pendingMessages[mid] = mes;
	} else{
		throw Error("message to put in pending already exist");
	}	
}
