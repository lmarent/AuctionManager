
/*! \file   AgentSession.h

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

    $Id: AgentSession.h 748 2015-10-01 18:49:00Z amarentes $
*/

#ifndef _AGENT_SESSION_H_
#define _AGENT_SESSION_H_

#include "Session.h"
#include "AuctionManager.h"


namespace auction
{


class AgentSession : public Session
{

public:
	
    /*! \short   This function creates a new object instance. 
        \returns a new object instance.
    */
	AgentSession( string _sessionId  );

	~AgentSession();

	void setStart(time_t _start);
	
	void setStop(time_t s_top);

    void addAuctions(auctionDB_t *auctions);
	
	auctionSetIter_t beginAuction(void);
	
	auctionSetIter_t endAuction(void);
	
	void setAuctions(auctionSet_t &setParam);
	
	auctionSet_t & getAuctions(void);
	
	string getInfo();
	
protected:
	
	//! establish the start time of the session 
	time_t start;
	
	//! Establish the end time of the session
	time_t stop;
	
	//! Auctions created in this session.
	auctionSet_t  auctionSet;


private:

    Logger *log;
    int ch; //!< logging channel number used by objects of this class


};

//! overload for <<, so that a Auctioner object can be thrown into an ostream
std::ostream& operator<< ( std::ostream &os, Session &obj );

}; // namespace auction



#endif // _AUM_SESSION_H_
