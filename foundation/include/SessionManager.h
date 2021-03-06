
/*! \file   SessionManager.h

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
    Resource request database
    Code based on Netmate Implementation

	$Id: SessionManager.h 748 2015-08-25 16:07:00Z amarentes $
*/

#ifndef _SESSION_MANAGER_H_
#define _SESSION_MANAGER_H_


#include "stdincpp.h"
#include "Logger.h"
#include "Error.h"
#include "Session.h"
#include "IdSource.h"
#include "EventScheduler.h"

namespace auction
{

typedef vector<Session *>						sessionDB_t;
typedef vector<Session *>::iterator				sessionDBIter_t;

typedef map<string, int>             				sessionIndex_t;
typedef map<string, int>::iterator    				sessionIndexIter_t;


//! list of done sessions
typedef list<Session*>            sessionDone_t;
typedef list<Session*>::iterator  sessionDoneIter_t;


/*! \short   manage adding/deleting of complete session descriptions
  
  the SessionManager class allows to add and remove sessions in the Auction
  core system. Session data are a set of ascii strings that are parsed
  and syntax checked by the SessionManager and then their respective
  settings are used to configure the other AuctionCore components. 
  The Session will then be stored in the sessionDatabase inside the SessionManager
*/

class SessionManager
{
  private:

    Logger *log;
    int ch; //!< logging channel number used by objects of this class

    //!< number of sessions in the database
    int sessions;

	//! session database.
	sessionDB_t sessionDB;
	
	//! index by sessioId
	sessionIndex_t sessionIndex;
	
	//! index by anslp session id
	sessionIndex_t sessionAnslpIndex;
	
    //! list with sessions done
    sessionDone_t sessionsDone;

    //! pool of unique sessions ids
    IdSource idSource;

    /*! 
	  \short add the session name to the list of finished sessions
       \arg \c r- Session to put in done.
    */
    void storeSessionAsDone(Session *s);

  public:


    /*! 
		\short   construct and initialize a SessionManager object
     */
    SessionManager(); 

    //! destroy a SessionManager object
    ~SessionManager(); // Ok

     /*! \short   lookup the Session info data for a given SessionId

        lookup the database of Sessions for a specific Session
        and return a link (pointer) to the Session data associated with it
        do not store this pointer, its contents will be destroyed upon Session deletion. 
        do not free this pointer as it is a link to the Session and not a copy.
    
        \arg \c uid - unique Session id
    */
    Session *getSession(int uid);

    //! get Session from sessionId 
    Session *getSession(string _sessionId);
	
	//! index active session
	void indexActiveSession(string sessionId, string anslpSessionId);
	
	//! get Session from anslp sessionid.
	Session *getAnslpSession(string anslpSessionId);
	
    //! get all sessions
    sessionDB_t getSessions();

    //! add a single Session
    void addSession(Session *s); 
    
    //! add a list of sessions.
    void addSessions(sessionDB_t * sessions, EventScheduler *e);

    /*! \short   delete a Session description 

        deletion of a Session will parse and syntax check the
        identification string, test the presence of the given Session
        in the SessionDatabase, remove the Session
        from the ResourceRequestDatabase

        \throws an Error exception if the given Session identification is not
        syntactically correct or does not contain the mandatory fields  or 
        if a Session with the given identification is currently not present 
        in the SessionDatabase
    */
    void delSession(int uid, EventScheduler *e); //ok
    void delSession(string _sessionId, EventScheduler *e); // ok
    void delSession(Session *s, EventScheduler *e);
    void delSessions(sessionDB_t *sessions, EventScheduler *e); //Ok
   
    /*! \short   get information from the Session manager

        these functions can be used to get information for a single Session,
        or a set of Sessions or all Sessions
    */
    string getInfo(void);

    inline string getInfo(int uid){ return getInfo(getSession(uid)); }

    string getInfo(Session *s);
    string getInfo(string _sessionId);

    //! Get the total number of sessions in the system.
    inline int getNumSessions(){ return sessions; }


    //! dump a SessionManager object
    void dump( ostream &os );

	Session * findSessionInStorage(string sessionId);
    
};


//! overload for <<, so that a SessionManager object can be thrown into an iostream
ostream& operator<< ( ostream &os, SessionManager &rm );

} // namespace auction

#endif // _SESSION_MANAGER_H_
