
/*! \file   AgentSessionManager.cpp

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
    Session database, this component besides storing sessions, creates new sessions.
    Code based on Netmate Implementation

	$Id: AgentSessionManager.cpp 748 2015-10-29 11:00:00Z amarentes $
*/

#include "ParserFcts.h"
#include "AgentSessionManager.h"
#include <uuid/uuid.h>



using namespace auction;


AgentSessionManager::AgentSessionManager(): 
	SessionManager()
{

}


AgentSessionManager::~AgentSessionManager()
{
	// Nothing to do.
}
		

AgentSession * 
AgentSessionManager::createAgentSession(string sessionId, string sourceAddr, string sSenderAddr, 
										string sDestinAddr, uint16_t senderPort, 
										uint16_t destinPort, uint8_t protocol, uint32_t lifetime )
{
	

	auction::AgentSession *session = new auction::AgentSession(sessionId);
	session->setSourceAddress(sourceAddr);
	session->setSenderAddress(sSenderAddr);
	session->setSenderPort(senderPort);
	session->setReceiverAddress(sDestinAddr);
	session->setSenderPort(destinPort);

	// Set the protocol which is defined by default.
	session->setProtocol(protocol);
	
	// Set the session's lifetime
	session->setLifetime(lifetime);
			
	return session;

	
}

AgentSession * 
AgentSessionManager::createAgentSession(string sourceAddr, string sSenderAddr, 
										string sDestinAddr, uint16_t senderPort, 
										uint16_t destinPort, uint8_t protocol, uint32_t lifetime )
{

	// typedef unsigned char uuid_t[16];
	uuid_t uuid;
	uuid_generate_time_safe(uuid);
	char uuid_str[37]; 
	uuid_unparse_lower(uuid, uuid_str); 
	std::string sessionId(uuid_str);
	uuid_clear(uuid);
	

	return createAgentSession(sessionId, sourceAddr, sSenderAddr, 
								sDestinAddr, senderPort, destinPort, protocol, lifetime );
	
}

