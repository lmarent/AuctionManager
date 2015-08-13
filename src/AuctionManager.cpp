
/*! \file   AuctionManager.cpp

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
	auction database
    Code based on Netmate Implementation

    $Id: AuctionManager.h 748 2015-08-04 13:50:00Z amarentes $

*/

#include "AuctionManager.h"
#include "CtrlComm.h"
#include "Constants.h"

/* ------------------------- AuctionManager ------------------------- */

AuctionManager::AuctionManager() 
    : auctions(0), 
	  idSource(1)
{
    log = Logger::getInstance();
    ch = log->createChannel("AuctionManager");
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

}


/* ------------------------- ~AuctionManager ------------------------- */

AuctionManager::~AuctionManager()
{
    auctionDBIter_t iter;

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif
    for (iter = auctionDB.begin(); iter != auctionDB.end(); iter++) {
        if (*iter != NULL) {
            // delete auction
            saveDelete(*iter);
        } 
    }
	
    for (auctionDoneIter_t i = auctionDone.begin(); i != auctionDone.end(); i++) {
        saveDelete(*i);
    }
}


/* -------------------- isReadableFile -------------------- */

static int isReadableFile( string fileName ) {

    FILE *fp = fopen(fileName.c_str(), "r");

    if (fp != NULL) {
        fclose(fp);
        return 1;
    } else {
        return 0;
    }
}

/* -------------------------- getAuction ----------------------------- */

Auction *AuctionManager::getAuction(int uid)
{
    if ((uid >= 0) && ((unsigned int)uid < auctionDB.size())) {
        return auctionDB[uid];
    } else {
        return NULL;
    }
}



/* -------------------- getAuction -------------------- */

Auction *AuctionManager::getAuction(string sname, string rname)
{
    auctionSetIndexIter_t iter;
    auctionIndexIter_t iter2;

    iter = auctionSetIndex.find(sname);
    if (iter != auctionSetIndex.end()) {		
        iter2 = iter->second.find(rname);
        if (iter2 != iter->second.end()) {
            return getAuction(iter2->second);
        }
        else
        {
#ifdef DEBUG
    log->dlog(ch,"Auction Id not found");
#endif		
			
		}
    }
    else
    {
#ifdef DEBUG
    log->dlog(ch,"Auction set not found");
#endif		
	}

    return NULL;
}


/* -------------------- getAuctions -------------------- */

auctionIndex_t *AuctionManager::getAuctions(string sname)
{
    auctionSetIndexIter_t iter;

    iter = auctionSetIndex.find(sname);
    if (iter != auctionSetIndex.end()) {
        return &(iter->second);
    }

    return NULL;
}

auctionDB_t  AuctionManager::getAuctions()
{
    auctionDB_t ret;

    for (auctionSetIndexIter_t r = auctionSetIndex.begin(); r != auctionSetIndex.end(); r++) {
        for (auctionIndexIter_t i = r->second.begin(); i != r->second.end(); i++) {
            ret.push_back(getAuction(i->second));
        }
    }

    return ret;
}

/* ----------------------------- parseAuctions --------------------------- */

auctionDB_t *AuctionManager::parseAuctions(string fname)
{

#ifdef DEBUG
    log->dlog(ch,"ParseAuctions");
#endif

    auctionDB_t *new_auctions = new auctionDB_t();

    try {	
	
        AuctionFileParser afp = AuctionFileParser(fname);
        afp.parse(new_auctions, &idSource);

#ifdef DEBUG
    log->dlog(ch, "Auctions parsed");
#endif

        return new_auctions;

    } catch (Error &e) {

        for(auctionDBIter_t i=new_auctions->begin(); i != new_auctions->end(); i++) {
           saveDelete(*i);
        }
        saveDelete(new_auctions);
        throw e;
    }
}


/* -------------------- parseBidsBuffer -------------------- */

auctionDB_t *AuctionManager::parseAuctionsBuffer(char *buf, int len, int mapi)
{
    auctionDB_t *new_auctions = new auctionDB_t();

    try {
			
        if (mapi) {
             MAPIAuctionParser afp = MAPIAuctionParser(buf, len);
             afp.parse(new_auctions, &idSource);
        } else {
            AuctionFileParser afp = AuctionFileParser(buf, len);
            afp.parse(new_auctions, &idSource);
        }

        return new_auctions;
	
    } catch (Error &e) {

        for(auctionDBIter_t i=new_auctions->begin(); i != new_auctions->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_auctions);
        throw e;
    }
}


/* ---------------------------------- addAuctions ----------------------------- */

void AuctionManager::addAuctions(auctionDB_t * _auctions, EventScheduler *e)
{
    auctionDBIter_t        iter;
    auctionTimeIndex_t     start;
    auctionTimeIndex_t     stop;
    auctionTimeIndexIter_t iter2;
    time_t              now = time(NULL);
    
    // add auctions
    for (iter = _auctions->begin(); iter != _auctions->end(); iter++) {
        Auction *a = (*iter);
        try {
            
            addAuction(a);
            start[a->getStart()].push_back(a);
            if (a->getStop()) 
            {
                stop[a->getStop()].push_back(a);
            }
        } catch (Error &e ) {
            saveDelete(a);
            // if only one rule return error
            if (_auctions->size() == 1) {
                throw e;
            }
            // FIXME else return number of successively installed bids
        }
      
    }
    
#ifdef DEBUG    
    log->dlog(ch, "Start all auctions - it is going to activate them");
#endif      

    // group auctions with same start time
    for (iter2 = start.begin(); iter2 != start.end(); iter2++) {
        e->addEvent(new ActivateAuctionsEvent(iter2->first-now, iter2->second));
    }
    
    // group auctions with same stop time
    for (iter2 = stop.begin(); iter2 != stop.end(); iter2++) {
        e->addEvent(new RemoveAuctionsEvent(iter2->first-now, iter2->second));
    }

#ifdef DEBUG    
    log->dlog(ch, "Finished adding auctions");
#endif      

}


/* -------------------- addAuction -------------------- */

void AuctionManager::addAuction(Auction *a)
{
  
#ifdef DEBUG    
    log->dlog(ch, "adding new auction with Id = %d, name = '%s'",
              a->getUId(), a->getAuctionName().c_str());
#endif  
				  
			  
    // test for presence of auctionSource/auctionName combination
    // in auctionDatabase in particular set
    if ( getAuction(a->getSetName(), a->getAuctionName()) != NULL ) {
        log->elog(ch, "Auction %s.%s already installed",
                  a->getSetName().c_str(), a->getAuctionName().c_str());
        throw Error(408, "Auction with this name is already installed");
    }
        		
    try {

		// Assigns the new Id.
		a->setUId(idSource.newId());

        // could do some more checks here
        a->setState(AS_VALID);

#ifdef DEBUG    
		log->dlog(ch, "Auction Id = '%d'", a->getUId());
#endif 

        // resize vector if necessary
        if ((unsigned int)a->getUId() >= auctionDB.size()) {
            auctionDB.reserve((a->getUId() * 2) + 1);
            auctionDB.resize(a->getUId() + 1);
        }
		
        // insert auction
        auctionDB[a->getUId()] = a; 	

        // add new entry in index
        auctionSetIndex[a->getSetName()][a->getAuctionName()] = a->getUId();
	
        auctions++;

#ifdef DEBUG    
    log->dlog(ch, "finish adding new auction with name = '%s'",
              a->getAuctionName().c_str());
#endif  

    } catch (Error &e) { 

        // adding new auction failed in some component
        // something failed -> remove auction from database
        delAuction(a->getSetName(), a->getAuctionName(), NULL);
	
        throw e;
    }
}

void AuctionManager::activateAuctions(auctionDB_t *auctions, EventScheduler *e)
{

#ifdef DEBUG    
    log->dlog(ch, "Activate auctions");
#endif  

    auctionDBIter_t             iter;
    auctionIntervalsIndexIter_t iter2;
    auctionIntervalsIndex_t     intervals;    

    for (iter = auctions->begin(); iter != auctions->end(); iter++) {
        Auction *a = (*iter);
        log->dlog(ch, "activate auction with name = '%s'", a->getAuctionName().c_str());
        a->setState(AS_ACTIVE);
		
		// Create the execution intervals
		intervalList_t *ilist = a->getIntervals();
        for (intervalListIter_t i = ilist->begin(); i != ilist->end(); i++) {
             procdef_t entry;

             entry.i = i->first;
             entry.e = i->second;
              
             intervals[entry].push_back(a);
        }
    }

    // group by export interval
    for (iter2 = intervals.begin(); iter2 != intervals.end(); iter2++) {
        unsigned long i = iter2->first.i.interval;
#ifdef DEBUG    
    log->dlog(ch, "Activate auctions - Execution interval: %lu", i );
#endif  

        e->addEvent(new PushExecutionEvent(i, iter2->second, iter2->first.e,
                                        i * 1000, iter2->first.i.align));
    }
    

}


/* ------------------------- getInfo ------------------------- */

string AuctionManager::getInfo(Auction *a)
{
    ostringstream s;

#ifdef DEBUG
    log->dlog(ch, "looking up Auction with uid = %d", a->getUId());
#endif

    s << a->getInfo();
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string AuctionManager::getInfo(string sname, string rname)
{
    ostringstream s;
    string info;
    Auction *a;
  
    a = getAuction(sname, rname);

    if (a == NULL) {
        // check done tasks
        for (auctionDoneIter_t i = auctionDone.begin(); i != auctionDone.end(); i++) {
            if (((*i)->getAuctionName() == rname) && ((*i)->getSetName() == sname)) {
                info = (*i)->getInfo();
            }
        }
        
        if (info.empty()) {
            throw Error("no auction with bid name '%s.%s'", sname.c_str(), rname.c_str());
        }
    } else {
        // rule with given identification is in database
        info = a->getInfo();
    }
    
    s << info;

    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string AuctionManager::getInfo(string sname)
{
    ostringstream s;
    auctionSetIndexIter_t a;

    a = auctionSetIndex.find(sname);

    if (a != auctionSetIndex.end()) {
        for (auctionIndexIter_t i = a->second.begin(); i != a->second.end(); i++) {
            s << getInfo(sname, i->first);
        }
    } else {
        s << "No such auction set" << endl;
    }
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string AuctionManager::getInfo()
{
    ostringstream s;
    auctionSetIndexIter_t iter;

    for (iter = auctionSetIndex.begin(); iter != auctionSetIndex.end(); iter++) {
        s << getInfo(iter->first);
    }
    
    return s.str();
}


/* ------------------------- delAuction ------------------------- */

void AuctionManager::delAuction(string sname, string rname, EventScheduler *e)
{
    Auction *a;

#ifdef DEBUG    
    log->dlog(ch, "Deleting auction set= %s name = '%s'",
              sname.c_str(), rname.c_str());
#endif  


    if (sname.empty() && rname.empty()) {
        throw Error("incomplete auction set or name specified");
    }

    a = getAuction(sname, rname);

    if (a != NULL) {
        delAuction(a, e);
    } else {
        throw Error("Auction %s.%s does not exist", sname.c_str(),rname.c_str());
    }
}


/* ------------------------- delAuction ------------------------- */

void AuctionManager::delAuction(int uid, EventScheduler *e)
{
    Auction *a;

    a = getAuction(uid);

    if (a != NULL) {
        delAuction(a, e);
    } else {
        throw Error("Auction uid %d does not exist", uid);
    }
}


/* ------------------------- delAuctions ------------------------- */

void AuctionManager::delAuctions(string sname, EventScheduler *e)
{

#ifdef DEBUG    
    log->dlog(ch, "removing auction with set = '%s'", sname.c_str());
#endif
    
    if (auctionSetIndex.find(sname) != auctionSetIndex.end()) {
        auctionSetIndexIter_t iter = auctionSetIndex.find(sname);
        auctionIndex_t auctionIndex = iter->second;
        for (auctionIndexIter_t i = auctionIndex.begin(); i != auctionIndex.end(); i++) {
						
            delAuction(getAuction(sname, i->first),e);
        }
    }
}


/* ------------------------- delAuction ------------------------- */

void AuctionManager::delAuction(Auction *a, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "removing auction with name = '%s'", a->getAuctionName().c_str());
#endif

    // remove auction from database and from index
    storeAuctionAsDone(a);
    auctionDB[a->getUId()] = NULL;
    auctionSetIndex[a->getSetName()].erase(a->getAuctionName());

    // delete auction set if empty
    if (auctionSetIndex[a->getSetName()].empty()) {
        auctionSetIndex.erase(a->getSetName());
    }
    
    if (e != NULL) {
        e->delAuctionEvents(a->getUId());
    }

    auctions--;
}


/* ------------------------- delAuctions ------------------------- */

void AuctionManager::delAuctions(auctionDB_t *_auctions, EventScheduler *e)
{
    auctionDBIter_t iter;

    for (iter = _auctions->begin(); iter != _auctions->end(); iter++) {
        delAuction(*iter, e);
    }
}


/* -------------------- storeAuctionAsDone -------------------- */

void AuctionManager::storeAuctionAsDone(Auction *a)
{
    
    a->setState(AS_DONE);
    auctionDone.push_back(a);

    if (auctionDone.size() > DONE_LIST_SIZE) {
        // release id
        idSource.freeId(auctionDone.front()->getUId());
        // remove auction
        saveDelete(auctionDone.front());
        auctionDone.pop_front();
    }
}


/* ------------------------- dump ------------------------- */

void AuctionManager::dump( ostream &os )
{
    
    os << "Auction Manager dump :" << endl;
    os << getInfo() << endl;
    
}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, AuctionManager &am )
{
    am.dump(os);
    return os;
}
