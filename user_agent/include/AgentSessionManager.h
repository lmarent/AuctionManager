
/*! \file   AgentSessionManager.h

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

	$Id: AgentSessionManager.h 748 2015-10-29 11:00:00Z amarentes $
*/

#ifndef _AGENT_SESSION_MANAGER_H_
#define _AGENT_SESSION_MANAGER_H_


namespace auction
{

class AgentSessionManager : public SessionManager
{

	public: 
		
		AgentSessionManager();
		
		~AgentSessionManager();
		
		/*! \short   Creates a new session with the same session Id given.
		 *			 Session with the same id mean sessions that belong to
		 * 			 the same request, but wuth different destination.

    
        \arg \c sessionId - Session id
        \arg \c sourceAddr - Source Address for the session
        \arg \c sSenderAddr - Sender address for the session
        \arg \c sDestinAddr - Destination address for the session
        \arg \c senderPort  - Sender port for the session
        \arg \c protocol    - Sender protocol for the session
        
		*/
		AgentSession * createAgentSession(string sessionId, string sourceAddr, string sSenderAddr, 
										  string sDestinAddr, uint16_t senderPort, 
										  uint16_t destinPort, uint8_t protocol, uint32_t lifetime );

		/*! \short   Creates a new session, it assigns the id for the session.
		 *			 Session with the same id mean sessions that belong to
		 * 			 the same request, but wuth different destination.
		  
        \arg \c sourceAddr - Source Address for the session
        \arg \c sSenderAddr - Sender address for the session
        \arg \c sDestinAddr - Destination address for the session
        \arg \c senderPort  - Sender port for the session
        \arg \c protocol    - Sender protocol for the session
        
		*/
		AgentSession * createAgentSession(string sourceAddr, string sSenderAddr, 
										  string sDestinAddr, uint16_t senderPort, 
										  uint16_t destinPort, uint8_t protocol, uint32_t lifetime );

	
};

} // namespace auction

#endif // __AGENT_SESSION_MANAGER_H_
