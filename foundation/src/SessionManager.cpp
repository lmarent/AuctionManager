
/*! \file   SessionManager.cpp

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
	resource request database
    Code based on Netmate Implementation

    $Id: SessionManager.cpp 748 2015-10-01 20:45:00Z amarentes $

*/

#include "ParserFcts.h"
#include "SessionManager.h"
#include "Constants.h"

using namespace auction;

/* ------------------------- SessionManager ------------------------- */

SessionManager::SessionManager( ) 
    : sessions(0), idSource(1)
{
    log = Logger::getInstance();
    ch = log->createChannel("SessionManager");
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

}


/* ------------------------- ~SessionManager ------------------------- */

SessionManager::~SessionManager()
{
    sessionDBIter_t iter;

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

    for (iter = sessionDB.begin(); iter != sessionDB.end(); iter++) {
        if (*iter != NULL) {
            // delete resource request
            saveDelete(*iter);
        } 
    }

    sessionDoneIter_t i;
    for ( i = sessionsDone.begin(); i != sessionsDone.end(); i++ ) {
        saveDelete(*i);
    }
}


/* --------------------- getSession ------------------------- */

Session *SessionManager::getSession(int uid)
{
    if ((uid >= 0) && ((unsigned int)uid <= sessionDB.size())) {
        return sessionDB[uid];
    } else {
        return NULL;
    }
}



/* -------------------- getsession -------------------- */

Session *SessionManager::getSession(string _sessionId)
{
    sessionIndexIter_t iter;

    iter = sessionIndex.find(_sessionId);
    if (iter != sessionIndex.end()) {		
        return getSession(iter->second);
    }
    else
    {
#ifdef DEBUG
    log->dlog(ch,"Session Id not found %s", _sessionId.c_str());
#endif		
	}

    return NULL;
}


sessionDB_t 
SessionManager::getSessions()
{
    sessionDB_t ret;

    sessionIndexIter_t i;
    for ( i = sessionIndex.begin(); i != sessionIndex.end(); i++) 
    {
            ret.push_back(getSession(i->second));
    }

    return ret;
}


/* ------------------------ addsessions -------------------------- */

void 
SessionManager::addSessions(sessionDB_t * sessions, EventScheduler *e)
{
    sessionDBIter_t        iter;
        
    // add Sessions
    for (iter = sessions->begin(); iter != sessions->end(); iter++) {
        Session *s = (*iter);
        
        try {

            addSession(s);
			
        } catch (Error &e ) {
            saveDelete(s);
            // if only one session return error
            if (sessions->size() == 1) {
                throw e;
            }
            // FIXME else return number of successively installed Sessions
        }
    }
    

#ifdef DEBUG    
    log->dlog(ch, "Finished adding sessions");
#endif      

}


/* -------------------- addSession -------------------- */

void 
SessionManager::addSession(Session *session)
{
  
#ifdef DEBUG    
    log->dlog(ch, "adding new sessions with id = '%s'", session->getSessionId().c_str());
#endif  
				  
			  
    // test for presence of Id in SessionDatabase in particular set
    if (getSession(session->getSessionId())) {
        log->elog(ch, "Session Id %s already installed", session->getSessionId().c_str());
        throw Error(408, "Session with this id is already installed");
    }

    try {

		// Assigns the new Id.
		session->setUId(idSource.newId());

        // could do some more checks here
        session->setState(SS_VALID);

#ifdef DEBUG    
		log->dlog(ch, "Session uId = '%d'", session->getUId());
#endif 

        // resize vector if necessary
        if ((unsigned int)session->getUId() >= sessionDB.size()) {
            sessionDB.reserve(session->getUId() * 2 + 1);
            sessionDB.resize(session->getUId() + 1);
        }

        // insert Session
        sessionDB[session->getUId()] = session; 	

        // add new entry in index
        sessionIndex[session->getSessionId()] = session->getUId();
	
        sessions++;

#ifdef DEBUG    
    log->dlog(ch, "finish adding new Session with Id = '%s'",
              session->getSessionId().c_str());
#endif  

    } catch (Error &e) { 

        // adding new Session failed in some component
        // something failed -> remove Session from database
        delSession(session->getSessionId(), NULL);
	
        throw e;
    }
}


/* ------------------------- getInfo ------------------------- */

string SessionManager::getInfo(Session *s)
{
    ostringstream os;

#ifdef DEBUG
    log->dlog(ch, "looking up Session with uid = %d", s->getUId());
#endif

    os << s->getInfo() << endl;
    
    return os.str();
}


/* ------------------------- getInfo ------------------------- */

string SessionManager::getInfo(string _sessionId)
{
    ostringstream os;
    string info;
    Session *s;
  
    s = getSession(_sessionId);

    if (s == NULL) {
        // check done tasks
        for (sessionDoneIter_t i = sessionsDone.begin(); i != sessionsDone.end(); i++) {
            if (((*i)->getSessionId() == _sessionId) ) {
                info = (*i)->getInfo();
            }
        }
        
        if (info.empty()) {
            throw Error("no Session with id:'%s'", _sessionId.c_str());
        }
    } else {
        // Session with given identification is in database
        info = s->getInfo();
    }
    
    os << info;

    return os.str();
}



/* ------------------------- getInfo ------------------------- */

string SessionManager::getInfo()
{
    ostringstream os;
    sessionIndexIter_t iter;

    for (iter = sessionIndex.begin(); iter != sessionIndex.end(); iter++) {
        os << getInfo(iter->first);
    }
    
    return os.str();
}

/* ---------------------- delSession ----------------------- */

void SessionManager::delSession(int uid, EventScheduler *e)
{
    Session *s;

    s = getSession(uid);

    if (s != NULL) {
        delSession(s, e);
    } else {
        throw Error("Session uid %d does not exist", uid);
    }
}


/* ------------------------- delSession ------------------------- */

void SessionManager::delSession(string _sessionId, EventScheduler *e)
{
    Session *s;

#ifdef DEBUG    
    log->dlog(ch, "Deleting Session id= %s", _sessionId.c_str());
#endif  


    if ( _sessionId.empty() ) 
    {
        throw Error("incomplete Session Id");
    }

    s = getSession(_sessionId);

    if (s != NULL) {
        delSession(s, e);
    } else {
        throw Error("Session %s does not exist", _sessionId.c_str());
    }
}

/* ------------------------- delBid ------------------------- */

void SessionManager::delSession(Session *s, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "removing Session with Id = '%s'", s->getSessionId().c_str());
#endif

	// remove Session from database and from index
	storeSessionAsDone(s);
	sessionDB[s->getUId()] = NULL;
	sessionIndex.erase(s->getSessionId());
		
	if (e != NULL) {
		e->delSessionEvents(s->getUId());
	}

	sessions--;

}


/* ----------------------- delSessions ----------------------- */

void SessionManager::delSessions(sessionDB_t *sessions, EventScheduler *e)
{
    sessionDBIter_t iter;

    for (iter = sessions->begin(); iter != sessions->end(); iter++) {
        delSession(*iter, e);
    }
}


/* -------------------- storeSessionAsDone -------------------- */

void SessionManager::storeSessionAsDone(Session *s)
{
    
    s->setState(SS_DONE);
    sessionsDone.push_back(s);

    if (sessionsDone.size() > DONE_LIST_SIZE) {
        // release id
        idSource.freeId(sessionsDone.front()->getUId());
        // remove Session
        saveDelete(sessionsDone.front());
        sessionsDone.pop_front();
    }
}


/* ------------------------- dump ------------------------- */

void SessionManager::dump( ostream &os )
{
    
    os << "SessionManager dump :" << endl;
    os << getInfo() << endl;
    
}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, SessionManager &rm )
{
    rm.dump(os);
    return os;
}
