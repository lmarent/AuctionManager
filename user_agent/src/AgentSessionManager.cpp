
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
	
	// Get the sender Address.
	field_t *fptr1 = getField("SrcIP");
	if (fptr1 != NULL){
		// TODO AM: implement more than one value.
		sSenderAddr = ((fptr1->value)[0]).getValue();
	}
	else {
		field_t *fptr2 = getField("SrcIP6");
		if (fptr2 != NULL){
			sSenderAddr = ((fptr2->value)[0]).getValue();
		}
	}
	session->setSenderAddress(sSenderAddr);

	// Get the sender Port.
	field_t *fptr3 = getField("SrcPort");
	if (fptr3 != NULL){
		// TODO AM: implement more than one value.
		string sSender_port = ((fptr3->value)[0]).getValue();
		unsigned short sndPort = (unsigned short) ParserFcts::parseULong(sSender_port);
		session->setSenderPort((uint16_t) sndPort);
	}
	else {
		session->setSenderPort(senderPort);
	}


	// Get the destination IP.
	field_t *fptr4 = getField("DstIP");
	if (fptr4 != NULL){
		// TODO AM: implement more than one value.
		sDestinAddr = ((fptr4->value)[0]).getValue();
	}
	else {
		field_t *fptr5 = getField("DstIP6");
		if (fptr5 != NULL){
			sDestinAddr = ((fptr5->value)[0]).getValue();
		}
	}
	session->setReceiverAddress(sDestinAddr);
				
	// get the destination PORT.
	field_t *fptr6 = getField("DstPort");
	if (fptr6 != NULL){
		// TODO AM: implement more than one value.
		string sDestin_port = ((fptr6->value)[0]).getValue();
		unsigned short dstPort = (unsigned short) ParserFcts::parseULong(sDestin_port);
		session->setReceiverPort((uint16_t) dstPort);
	}
	else {
		session->setSenderPort(destinPort);
	}

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

