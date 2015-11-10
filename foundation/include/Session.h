
/*! \file   Session.h

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

    Description:
    Sessions in the auction system

    $Id: AUMSession.h 748 2015-10-01 18:49:00Z amarentes $
*/

#ifndef _SESSION_H_
#define _SESSION_H_

#include "stdincpp.h"
#include "Logger.h"
#include "ConfigParser.h"
#include "address.h"
#include "session_id.h"
#include "MessageIdSource.h"

namespace auction
{


//! Session's states during lifecycle
typedef enum
{
    SS_NEW = 0,
    SS_VALID,
    SS_ACTIVE,
    SS_DONE,
    SS_ERROR
} sessionState_t;


typedef map<uint32_t, ipap_message> 					pendingMessageList_t;
typedef map<uint32_t, ipap_message>::iterator 			pendingMessageListIter_t;
typedef map<uint32_t, ipap_message>::const_iterator 	pendingMessageListConstIter_t;


class Session
{

public:
	
    /*! \short   This function creates a new object instance. 
        \returns a new object instance.
    */
	Session( string _sessionId  );

	virtual ~Session();

    int getUId() 
    { 
        return uid;
    }
    
    void setUId(int nuid)
    {
        uid = nuid;
    }

    void setState(sessionState_t s) 
    { 
        state = s;
    }

    sessionState_t getState()
    {
        return state;
    }
	
	void setSessionId(string _sessionId)
	{
		sessionId = _sessionId;
	}
	
	string getSessionId() const
	{
		return sessionId;
	}
	
	string getInfo();


	//! Set sender address 
	void setSenderAddress(string _sender_address);

	//! Get sender address
	protlib::hostaddress getSenderAddress();

	//! Set receiver address 
	void setReceiverAddress(string _receiver_address);

	//! Get receiver address 
	protlib::hostaddress getReceiverAddress();

	//! Set source address 
	void setSourceAddress(string _source_address);

	//! Get source address 
	protlib::hostaddress getSourceAddress();

	//! Set sender port
	void setSenderPort(uint16_t _sender_port);

	//! Get sender port
	uint16_t getSenderPort();

	//! Set receiver port
	void setReceiverPort(uint16_t _receiver_port);

	//! Get receiver port
	uint16_t getReceiverPort();

	//! Set protocol
	void setProtocol(uint8_t protocol);
	
	//! Get protocol
	uint8_t getProtocol();

	//! Set lifetime
	void setLifetime(uint32_t lifetime);
	
	//! Get lifetime
	uint32_t getLifetime();
	
	uint32_t getNextMessageId();
	
	//! Set a-nslp application sessionId 
	void setAnlspSession(anslp::session_id _sid);	
	
	anslp::session_id getAnlspSession(){ return sid; }
		
	//! Confirm the response of the message with id mid.
	void confirmMessage(uint32_t mid);
	
	//! add to the list of pensing message a new entry.
	void addPendingMessage(ipap_message mes);
	
	pendingMessageListIter_t beginMessages() { return pendingMessages.begin(); }		
	
	pendingMessageListIter_t endMessages() { return pendingMessages.end(); }		
		
protected:
	
    //! unique internal sessionID for this Session instance (has to be provided)
    int uid;
   
	//! state of this rule
    sessionState_t state;

	//! Session Id.
	string sessionId;

	//! Session id in the a-nslp application
	anslp::session_id sid;

	//! List of messages pending for confirmation.
	pendingMessageList_t pendingMessages;
		  
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

	//! Sequence number for messages being transfered from this session.
	MessageIdSource mId;
	
};

//! overload for <<, so that a Auctioner object can be thrown into an ostream
std::ostream& operator<< ( std::ostream &os, Session &obj );

}; // namespace auction



#endif // _AUM_SESSION_H_
