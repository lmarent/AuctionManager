
/*! \file   AuctioningObjectManager.cpp

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
	auction object database
    Code based on Netmate Implementation

    $Id: AuctioningObjectManager.h 748 2016-04-28 16:45:00Z amarentes $

*/

#include "ParserFcts.h"
#include "AuctioningObjectManager.h"
#include "Constants.h"

using namespace auction;

/* ------------------------- AuctioningObjectManager ------------------------- */

AuctioningObjectManager::AuctioningObjectManager( int domain, string fdname, string fvname, string channelName) 
    : FieldDefManager(fdname, fvname), objects(0), domain(domain), idSource(1)
{
    log = Logger::getInstance();
    ch = log->createChannel(channelName);
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif
	
}


/* ------------------------- ~AuctioningObjectManager ------------------------- */

AuctioningObjectManager::~AuctioningObjectManager()
{
    auctioningObjectDBIter_t iter;

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

    for (iter = auctioningObjectDB.begin(); iter != auctioningObjectDB.end(); iter++) {
        if (*iter != NULL) {
            // delete auction Object
            saveDelete(*iter);
        } 
    }
	
    for (auctioningObjectDoneIter_t i = auctioningObjectDone.begin(); i != auctioningObjectDone.end(); i++) {
        saveDelete(*i);
    }

#ifdef DEBUG
    log->dlog(ch,"Finish shutdown");
#endif

}


/* -------------------------- getAuctioningObject ---------------------- */

AuctioningObject *AuctioningObjectManager::getAuctioningObject(int uid)
{
    if ((uid >= 0) && ((unsigned int)uid < auctioningObjectDB.size())) {
        return auctioningObjectDB[uid];
    } else {
        return NULL;
    }
}



/* ------------------------- getAuctioningObject -------------------- */

AuctioningObject *AuctioningObjectManager::getAuctioningObject(string sname, string rname)
{
    auctioningObjectSetIndexIter_t iter;
    auctioningObjectIndexIter_t iter2;

    iter = auctioningObjectSetIndex.find(sname);
    if (iter != auctioningObjectSetIndex.end()) {		
        iter2 = iter->second.find(rname);
        if (iter2 != iter->second.end()) {
            return getAuctioningObject(iter2->second);
        }
        else
        {
#ifdef DEBUG
    log->dlog(ch,"Auction Object Id not found %s", rname.c_str());
#endif		
			
		}
    }
    else
    {
#ifdef DEBUG
    log->dlog(ch,"Auction Object set not found %s", sname.c_str());
#endif		
	}

    return NULL;
}


/* ------------------------ getAuctionObjects -------------------- */

auctioningObjectIndex_t *AuctioningObjectManager::getAuctioningObjects(string sname)
{
    auctioningObjectSetIndexIter_t iter;

    iter = auctioningObjectSetIndex.find(sname);
    if (iter != auctioningObjectSetIndex.end()) {
        return &(iter->second);
    }

    return NULL;
}

auctioningObjectDB_t  AuctioningObjectManager::getAuctioningObjects()
{
    auctioningObjectDB_t ret;

    for (auctioningObjectSetIndexIter_t r = auctioningObjectSetIndex.begin(); 
				r != auctioningObjectSetIndex.end(); r++) 
	{
					
        for (auctioningObjectIndexIter_t i = r->second.begin(); i != r->second.end(); i++) {
            ret.push_back(getAuctioningObject(i->second));
        }
    }

    return ret;
}


void AuctioningObjectManager::activateAuctioningObjects(auctioningObjectDB_t *auctioningObjects)
{
    auctioningObjectDBIter_t             iter;

    for (iter = auctioningObjects->begin(); iter != auctioningObjects->end(); iter++) {
        AuctioningObject *a = (*iter);
        log->dlog(ch, "activate auctioning object with name = %s.%s", 
						a->getSet().c_str(), a->getName().c_str());
						
        a->setState(AO_ACTIVE);
    }
}

/* -------------------- addAuctionObject -------------------- */

void AuctioningObjectManager::addAuctioningObject(AuctioningObject *a)
{
	
	assert(a != NULL);
	
#ifdef DEBUG    
    log->dlog(ch, "adding new auction with set = %s, name = '%s'",
              a->getSet().c_str(), a->getName().c_str());
#endif  
				  
			  
    // test for presence of set/Name combination
    // in auction Object Database
    if ( getAuctioningObject(a->getSet(), a->getName()) != NULL ) {
        log->elog(ch, "Auction Object %s.%s already installed",
                  a->getSet().c_str(), a->getName().c_str());
        throw Error(408, "Auctioning Object with this name is already installed");
    }
        		
    try {

		// Assigns the new Id.
		a->setUId(idSource.newId());

        // could do some more checks here
        a->setState(AO_VALID);

#ifdef DEBUG    
		log->dlog(ch, "Auctioning Object Id = #%d ", a->getUId());
#endif 

        // resize vector if necessary
        if ((unsigned int)a->getUId() >= auctioningObjectDB.size()) {
            auctioningObjectDB.reserve((a->getUId() * 2) + 1);
            auctioningObjectDB.resize(a->getUId() + 1);
        }
		
        // insert auctioning Object
        auctioningObjectDB[a->getUId()] = a; 	

        // add new entry in index
        auctioningObjectSetIndex[a->getSet()][a->getName()] = a->getUId();
	
        objects++;

#ifdef DEBUG    
    log->dlog(ch, "finish adding new auctioning objects with name = %s.%s",
              a->getSet().c_str(), a->getName().c_str() );
#endif  

    } catch (Error &e) { 

        // adding new auction object failed in some component
        // something failed -> remove auction object from database
        delAuctioningObject(a->getSet(), a->getName());
	
        throw e;
    }
}




AuctioningObject *
AuctioningObjectManager::getAuctioningObjectDone(string sname, string rname)
{

#ifdef DEBUG
    log->dlog(ch,"get Auctioning Object from Done %s.%s", sname.c_str(), rname.c_str());
#endif		


    for (auctioningObjectDoneIter_t i = auctioningObjectDone.begin(); i != auctioningObjectDone.end(); i++) {
       if (((*i)->getName() == rname) && ((*i)->getSet() == sname)) {
           return *i;
       }
	}
	
	return NULL;
}


AuctioningObject *
AuctioningObjectManager::getAuctioningObjectDone(int uid)
{

#ifdef DEBUG
    log->dlog(ch,"get Auctioning Object from Done %d", uid);
#endif		


    for (auctioningObjectDoneIter_t i = auctioningObjectDone.begin(); i != auctioningObjectDone.end(); i++) {
       if ((*i)->getUId() == uid) {
           return *i;
       }
	}
	
	return NULL;
}



/* ------------------------- delAuctioningObject ------------------------- */

void AuctioningObjectManager::delAuctioningObject(string sname, string rname)
{
    AuctioningObject *a;

#ifdef DEBUG    
    log->dlog(ch, "Deleting auctioning object %s.%s", sname.c_str(), rname.c_str());
#endif  


    if (sname.empty() && rname.empty()) {
        throw Error("incomplete auction object set or name not specified");
    }

    a = getAuctioningObject(sname, rname);

    if (a != NULL) {
        delAuctioningObject(a);
    } else {
        throw Error("Auctioning Object %s.%s does not exist", sname.c_str(),rname.c_str());
    }
}


/* ------------------------- delAuctioningObject ------------------------- */

void AuctioningObjectManager::delAuctioningObject(int uid)
{
    AuctioningObject *a;

    a = getAuctioningObject(uid);

    if (a != NULL) {
        delAuctioningObject(a);
    } else {
        throw Error("Auctioning Object uid %d does not exist", uid);
    }
}


/* ------------------------- delAuctioningObjects ------------------------- */

void 
AuctioningObjectManager::delAuctioningObjects(string sname)
{

#ifdef DEBUG    
    log->dlog(ch, "removing auctioning objects with set = '%s'", sname.c_str());
#endif
    
    if (auctioningObjectSetIndex.find(sname) != auctioningObjectSetIndex.end()) 
    {
        auctioningObjectSetIndexIter_t iter = auctioningObjectSetIndex.find(sname);    
        auctioningObjectIndex_t auctionIndex = iter->second;
        
        for (auctioningObjectIndexIter_t i = auctionIndex.begin(); i != auctionIndex.end(); i++) {
						
            delAuctioningObject(getAuctioningObject(sname, i->first));
        }
    }
}


/* ------------------------- delAuctioningObject ------------------------- */

void AuctioningObjectManager::delAuctioningObject(AuctioningObject *a)
{
#ifdef DEBUG    
    log->dlog(ch, "removing auctioning object with name = %s.%s", a->getSet().c_str(), a->getName().c_str());
#endif

    // remove auction from database and from index
    storeAuctioningObjectAsDone(a);
    auctioningObjectDB[a->getUId()] = NULL;
    auctioningObjectSetIndex[a->getSet()].erase(a->getName());

    // delete auction set if empty
    if (auctioningObjectSetIndex[a->getSet()].empty()) {
        auctioningObjectSetIndex.erase(a->getSet());
    }
    
    objects--;
}



/* -------------------- storeAuctionObjectAsDone -------------------- */

void AuctioningObjectManager::storeAuctioningObjectAsDone(AuctioningObject *a)
{
    
    a->setState(AO_DONE);
    auctioningObjectDone.push_back(a);

    if (auctioningObjectDone.size() > DONE_LIST_SIZE) {
        // release id
        idSource.freeId(auctioningObjectDone.front()->getUId());
        
        // remove auctioning object
        AuctioningObject *ao = auctioningObjectDone.front();
        delete ao;
        auctioningObjectDone.pop_front();
    }
}



