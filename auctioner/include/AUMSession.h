
/*! \file   AUMSession.h

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

#ifndef _AUM_SESSION_H_
#define _AUM_SESSION_H_

#include "stdincpp.h"
#include "Logger.h"
#include "ConfigParser.h"

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


class AUMSession
{

public:
	
    /*! \short   This function creates a new object instance. 
        \returns a new object instance.
    */
	AUMSession( string _sessionId  );

	~AUMSession();

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
	
	string getSessionId()
	{
		return sessionId;
	}
	
	string getInfo();
		
		
protected:
	
    //! unique internal sessionID for this Session instance (has to be provided)
    int uid;
   
	//! state of this rule
    sessionState_t state;

	//! Session Id.
	string sessionId;

};

}; // namespace auction



#endif // _AUM_SESSION_H_
