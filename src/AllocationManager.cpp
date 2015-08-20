
/*! \file   AllocationManager.cpp

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
	allocation database
    Code based on Netmate Implementation

    $Id: AllocationManager.h 748 2015-07-20 17:36:00Z amarentes $

*/

#include "AllocationManager.h"
#include "CtrlComm.h"
#include "Constants.h"

/* ------------------------- AllocationManager ------------------------- */

AllocationManager::AllocationManager( string fdname) 
    : bids(0), fieldDefFileName(fdname), idSource(1)
{
    log = Logger::getInstance();
    ch = log->createChannel("AllocationManager");

#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

}


/* ------------------------- ~AllocationManager ------------------------- */

AllocationManager::~AllocationManager()
{
    allocationDBIter_t iter;

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

    for (iter = allocationDB.begin(); iter != allocationDB.end(); iter++) {
        if (*iter != NULL) {
            // delete allocation
            saveDelete(*iter);
        } 
    }

    for (allocationDoneIter_t i = allocationDone.begin(); i != allocationDone.end(); i++) {
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

/* -------------------- loadFieldDefs -------------------- */

void AllocationManager::loadFieldDefs(string fname)
{
    if (fieldDefFileName.empty()) {
        if (fname.empty()) {
            fname = FIELDDEF_FILE;
		}
    } else {
        fname = fieldDefFileName;
    }

#ifdef DEBUG
    log->dlog(ch, "filename %s", fname.c_str());
#endif

    if (isReadableFile(fname)) {
        if (fieldDefs.empty() && !fname.empty()) {
            FieldDefParser f = FieldDefParser(fname.c_str());
            f.parse(&fieldDefs);
        }
    
    }else{
#ifdef DEBUG
    log->dlog(ch, "filename %s is not readable", fname.c_str());
#endif    
    }
    
}

/* -------------------------- getRule ----------------------------- */

Allocation *AllocationManager::getAllocation(int uid)
{
    if ((uid >= 0) && ((unsigned int)uid <= allocationDB.size())) {
        return allocationDB[uid];
    } else {
        return NULL;
    }
}



/* -------------------- getBid -------------------- */

Allocation *AllocationManager::getAllocaton(string aset, string aname, 
											string bset, string bname)
{
    allocationSetIndexIter_t iter;
    allocationIndexIter_t iter2;

    iter = bidSetIndex.find(bset);
    if (iter != bidSetIndex.end()) {		
        iter2 = iter->second.find(bname);
        if (iter2 != iter->second.end())
        {
			// Search in all bids referenced by the one with auction set and name 
			// given as parameter.
			vector<int>::iterator list_inter;
			for (list_inter = iter2->second.begin(); list_inter != iter->second.end(); ++list_inter)
			{
				Allocation * allocation = getAllocation(*list_inter);
				if ( aset.compare(allocation->getAuctionSet() ) == 0 ) &&
				    ( aname.compare(allocation->getAuctionName() ) == 0 )
				{
				    return  allocation;
				}  
			}
#ifdef DEBUG
			log->dlog(ch,"Auction not found %s.%s", aset.c_str(), aname.c_str());
#endif			
		}
		else
		{
		
#ifdef DEBUG
    log->dlog(ch,"Bid Name not found %s.%s", bset.c_str(), bname.c_str());
#endif		
			
		}
    }
    else
    {
#ifdef DEBUG
    log->dlog(ch,"Bidset not found %s", bset.c_str());
#endif		
	}

    return NULL;
}


allocationDB_t AllocationManager::getAllocations()
{
    allocationDB_t ret;

    allocationDBIter_t iter;
    for (iter = allocationDB.begin(); iter != allocationDB.end(); iter++) {
       ret.push_back(*iter);
    }

    return ret;
}

/* ---------------------------------- addBids ----------------------------- */

void AllocationManager::addAllocations(allocationDB_t * _allocations, EventScheduler *e)
{
    allocationDBIter_t        iter;
    allocationTimeIndex_t     start;
    allocationTimeIndex_t     stop;
    allocationTimeIndexIter_t iter2;
    time_t              now = time(NULL);
    
    // add allocations
    for (iter = _allocations->begin(); iter != _allocations->end(); iter++) {
        Allocation *a = (*iter);
        
        try {

            addAllocation(a);

            allocationIntervalListIter_t inter_iter;
            for ( inter_iter = (a->getIntervals())->begin(); 
                    inter_iter != (a->getIntervals())->end(); ++inter_iter ){ 
				start[inter_iter->start].push_back(a);
				if (inter_iter->stop) 
				{
					stop[inter_iter->stop].push_back(a);
				}
			}
        } catch (Error &e ) {
            saveDelete(a);
            // if only one allocation return error
            if (_bids->size() == 1) {
                throw e;
            }
            // FIXME else return number of successively installed allocations
        }
      
    }
    
#ifdef DEBUG    
    log->dlog(ch, "Start all allocations - it is going to activate them");
#endif      

    // group rules with same start time
    for (iter2 = start.begin(); iter2 != start.end(); iter2++) {
		allocationDBIter_t allocs_iter;
		// Iterates over the allocations starting.
		for (allocs_iter = (iter2->second).begin(); 
				allocs_iter != (iter2->second).end(); allocs_iter++) { 
			Allocation *alloc = (*allocs_iter);
			
					
#ifdef DEBUG    
			log->dlog(ch, "Schedulling new event - set: %s, name:  %s", 
 					  (inter_iter->auctionSet).c_str(), 
				    (inter_iter->auctionName).c_str());
#endif 					
			e->addEvent(new InsertAllocationEvent(iter2->first-now, alloc);
		}
    }
    
    // group allocations with same stop time
    for (iter2 = stop.begin(); iter2 != stop.end(); iter2++) {
		allocationDBIter_t allocs_iter;
		// Iterates over the allocations stoping.
		for (allocs_iter = (iter2->second).begin(); 
				allocs_iter != (iter2->second).end(); allocs_iter++) {
			Allocation *alloc = (*allocs_iter); 

			e->addEvent(new RemoveBidAuctionEvent(iter2->first-now, alloc );
		}
    }

#ifdef DEBUG    
    log->dlog(ch, "Finished adding allocations");
#endif      

}


/* -------------------- addAllocation -------------------- */

void AllocationManager::addAllocation(Allocation *a)
{
  
#ifdef DEBUG    
    log->dlog(ch, "adding new allocation with name = %s.%s.%s.%s",
              a->getAuctionSet().c_str(), 
              a->getAuctionName().c_str(),
              a->getBidSet().c_str(),
              a->getBidName().c_str());
#endif  
				  
			  
    // test for presence of allocationSource/allocationName combination
    // in allocation database in particular set
    if (getBid(b->getSetName(), b->getBidName())) {
        log->elog(ch, "bid %s.%s already installed",
                  b->getSetName().c_str(), b->getBidName().c_str());
        throw Error(408, "Bid with this name is already installed");
    }

    try {

		// Assigns the new Id.
		b->setUId(idSource.newId());

        // could do some more checks here
        b->setState(BS_VALID);

#ifdef DEBUG    
		log->dlog(ch, "Bid Id = '%d'", b->getUId());
#endif 

        // resize vector if necessary
        if ((unsigned int)b->getUId() >= bidDB.size()) {
            bidDB.reserve(b->getUId() * 2 + 1);
            bidDB.resize(b->getUId() + 1);
        }

        // insert bid
        bidDB[b->getUId()] = b; 	

        // add new entry in index
        bidSetIndex[b->getSetName()][b->getBidName()] = b->getUId();
	
        bids++;

#ifdef DEBUG    
    log->dlog(ch, "finish adding new bid with name = '%s'",
              b->getBidName().c_str());
#endif  

    } catch (Error &e) { 

        // adding new bid failed in some component
        // something failed -> remove bid from database
        delBid(b->getSetName(), b->getBidName(), NULL);
	
        throw e;
    }
}

void BidManager::activateBids(bidDB_t *bids, EventScheduler *e)
{
    bidDBIter_t             iter;

    for (iter = bids->begin(); iter != bids->end(); iter++) {
        Bid *r = (*iter);
        log->dlog(ch, "activate bid with name = '%s'", r->getBidName().c_str());
        r->setState(BS_ACTIVE);
	 
        /* TODO AM: Evaluate this code to understand if it has to be adjusted or not
        // set flow timeout
        if (r->isFlagEnabled(RULE_FLOW_TIMEOUT)) {
            unsigned long timeout = r->getFlowTimeout();
            if (timeout == 0) {
                // use the default
                timeout = FLOW_IDLE_TIMEOUT;
            }
            // flow timeout for flow based reporting (1 event per rule!)
			if (r->isFlagEnabled(RULE_AUTO_FLOWS)) {
				// check every second because we don't wanna readjust the event based on the
				// auto flows last packets, this would potentially mean lots of timeout events
				// expiring each second (however with this approach we might have an error of almost 1 s)
				e->addEvent(new FlowTimeoutEvent((unsigned long)1, r->getUId(), timeout, (unsigned long)1000));
			} else {
				// try to optimize the timeout checking, only check every timeout seconds
				// and readjust event based on last packet timestamp
				e->addEvent(new FlowTimeoutEvent(timeout, r->getUId(), timeout, timeout*1000));
			}
        }*/
    }

}


/* ------------------------- getInfo ------------------------- */

string BidManager::getInfo(Bid *r)
{
    ostringstream s;

#ifdef DEBUG
    log->dlog(ch, "looking up Bid with uid = %d", r->getUId());
#endif

    s << r->getInfo() << endl;
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string BidManager::getInfo(string sname, string rname)
{
    ostringstream s;
    string info;
    Bid *r;
  
    r = getBid(sname, rname);

    if (r == NULL) {
        // check done tasks
        for (bidDoneIter_t i = bidDone.begin(); i != bidDone.end(); i++) {
            if (((*i)->getBidName() == rname) && ((*i)->getSetName() == sname)) {
                info = (*i)->getInfo();
            }
        }
        
        if (info.empty()) {
            throw Error("no bid with bid name '%s.%s'", sname.c_str(), rname.c_str());
        }
    } else {
        // rule with given identification is in database
        info = r->getInfo();
    }
    
    s << info;

    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string BidManager::getInfo(string sname)
{
    ostringstream s;
    bidSetIndexIter_t b;

    b = bidSetIndex.find(sname);

    if (b != bidSetIndex.end()) {
        for (bidIndexIter_t i = b->second.begin(); i != b->second.end(); i++) {
            s << getInfo(sname, i->first);
        }
    } else {
        s << "No such bidset" << endl;
    }
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string BidManager::getInfo()
{
    ostringstream s;
    bidSetIndexIter_t iter;

    for (iter = bidSetIndex.begin(); iter != bidSetIndex.end(); iter++) {
        s << getInfo(iter->first);
    }
    
    return s.str();
}


/* ------------------------- delBid ------------------------- */

void BidManager::delBid(string sname, string rname, EventScheduler *e)
{
    Bid *r;

#ifdef DEBUG    
    log->dlog(ch, "Deleting bid set= %s name = '%s'",
              sname.c_str(), rname.c_str());
#endif  


    if (sname.empty() && rname.empty()) {
        throw Error("incomplete rule set or name specified");
    }

    r = getBid(sname, rname);

    if (r != NULL) {
        delBid(r, e);
    } else {
        throw Error("bid %s.%s does not exist", sname.c_str(),rname.c_str());
    }
}


/* ------------------------- delBid ------------------------- */

void BidManager::delBid(int uid, EventScheduler *e)
{
    Bid *r;

    r = getBid(uid);

    if (r != NULL) {
        delBid(r, e);
    } else {
        throw Error("bid uid %d does not exist", uid);
    }
}


/* ------------------------- delBids ------------------------- */

void BidManager::delBids(string sname, EventScheduler *e)
{
    
    if (bidSetIndex.find(sname) != bidSetIndex.end()) 
    {
		bidSetIndexIter_t iter = bidSetIndex.find(sname);
		bidIndex_t bidIndex = iter->second;
        for (bidIndexIter_t i = bidIndex.begin(); i != bidIndex.end(); i++) 
        {
            delBid(getBid(sname, i->first),e);
        }
    }
}


/* ------------------------- delBid ------------------------- */

void BidManager::delBid(Bid *r, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "removing bid with name = '%s'", r->getBidName().c_str());
#endif

    // remove bid from database and from index
    storeBidAsDone(r);
    bidDB[r->getUId()] = NULL;
    bidSetIndex[r->getSetName()].erase(r->getBidName());

    // delete bid set if empty
    if (bidSetIndex[r->getSetName()].empty()) {
        bidSetIndex.erase(r->getSetName());
    }
    
    if (e != NULL) {
        e->delBidEvents(r->getUId());
    }

    bids--;
}


/* ------------------------- delBids ------------------------- */

void BidManager::delBids(bidDB_t *bids, EventScheduler *e)
{
    bidDBIter_t iter;

    for (iter = bids->begin(); iter != bids->end(); iter++) {
        delBid(*iter, e);
    }
}


/* -------------------- storeBidAsDone -------------------- */

void BidManager::storeBidAsDone(Bid *r)
{
    
    r->setState(BS_DONE);
    bidDone.push_back(r);

    if (bidDone.size() > DONE_LIST_SIZE) {
        // release id
        idSource.freeId(bidDone.front()->getUId());
        // remove rule
        saveDelete(bidDone.front());
        bidDone.pop_front();
    }
}


/* ------------------------- dump ------------------------- */

void BidManager::dump( ostream &os )
{
    
    os << "BidManager dump :" << endl;
    os << getInfo() << endl;
    
}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, BidManager &rm )
{
    rm.dump(os);
    return os;
}
