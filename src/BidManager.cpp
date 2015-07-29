
/*! \file   BidManager.cpp

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
	bid database
    Code based on Netmate Implementation

    $Id: BidManager.h 748 2015-07-23 14:00:00Z amarentes $

*/

#include "BidManager.h"
#include "CtrlComm.h"
#include "constants.h"

/* ------------------------- BidManager ------------------------- */

BidManager::BidManager( ) 
    : tasks(0), 
	  idSource(1)
{
    log = Logger::getInstance();
    ch = log->createChannel("BidManager");
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

}


/* ------------------------- ~BidManager ------------------------- */

BidManager::~BidManager()
{
    bidDBIter_t iter;

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

    for (iter = bidDB.begin(); iter != bidDB.end(); iter++) {
        if (*iter != NULL) {
            // delete rule
            saveDelete(*iter);
        } 
    }

    for (bidDoneIter_t i = bidDone.begin(); i != bidDone.end(); i++) {
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


/* -------------------------- getRule ----------------------------- */

Bid *RuleManager::getRule(int uid)
{
    if ((uid >= 0) && ((unsigned int)uid <= ruleDB.size())) {
        return ruleDB[uid];
    } else {
        return NULL;
    }
}


/* -------------------- getBid -------------------- */

Rule *BidManager::getBid(string sname, string rname)
{
    bidSetIndexIter_t iter;
    bidIndexIter_t iter2;

    iter = bidSetIndex.find(sname);
    if (iter != bidSetIndex.end()) {		
        iter2 = iter->second.find(rname);
        if (iter2 != iter->second.end()) {
            return getBid(iter2->second);
        }
        else
        {
#ifdef DEBUG
    log->dlog(ch,"BidId not found");
#endif		
			
		}
    }
    else
    {
#ifdef DEBUG
    log->dlog(ch,"Bidset not found");
#endif		
	}

    return NULL;
}


/* -------------------- getBids -------------------- */

bidIndex_t *BidManager::getBids(string sname)
{
    bidSetIndexIter_t iter;

    iter = bidSetIndex.find(sname);
    if (iter != bidSetIndex.end()) {
        return &(iter->second);
    }

    return NULL;
}

bidDB_t BidManager::getBids()
{
    bidDB_t ret;

    for (bidSetIndexIter_t r = bidSetIndex.begin(); r != bidSetIndex.end(); r++) {
        for (bidIndexIter_t i = r->second.begin(); i != r->second.end(); i++) {
            ret.push_back(getBid(i->second));
        }
    }

    return ret;
}

/* ----------------------------- parseBids --------------------------- */

bidDB_t *BidManager::parseBids(string fname)
{

#ifdef DEBUG
    log->dlog(ch,"ParseBids");
#endif

    bidDB_t *new_bids = new bidDB_t();

    try {	
	
        BidFileParser rfp = BidFileParser(fname);
        rfp.parse(new_bids, &idSource);

#ifdef DEBUG
    log->dlog(ch, "bids parsed");
#endif

        return new_bids;

    } catch (Error &e) {

        for(bidDBIter_t i=new_bids->begin(); i != new_bids->end(); i++) {
           saveDelete(*i);
        }
        saveDelete(new_bids);
        throw e;
    }
}


/* -------------------- parseBidsBuffer -------------------- */

bidDB_t *BidManager::parseBidsBuffer(char *buf, int len, int mapi)
{
    bidDB_t *new_bids = new bidDB_t();

    try {
			
        if (mapi) {
             MAPIRuleParser rfp = MAPIBidParser(buf, len);
             rfp.parse(new_bids, &idSource);
        } else {
            BidFileParser rfp = BidFileParser(buf, len);
            rfp.parse(new_bids, &idSource);
        }

        return new_bids;
	
    } catch (Error &e) {

        for(bidDBIter_t i=new_bids->begin(); i != new_bids->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_bids);
        throw e;
    }
}


/* ---------------------------------- addRules ----------------------------- */

void BidManager::addBids(bidDB_t *rules, EventScheduler *e)
{
    bidDBIter_t        iter;
    bidTimeIndex_t     start;
    bidTimeIndex_t     stop;
    bidTimeIndexIter_t iter2;
    time_t              now = time(NULL);
    
    // add bids
    for (iter = bids->begin(); iter != bids->end(); iter++) {
        Bid *r = (*iter);
        
        try {
            addBid(r);

            start[r->getStart()].push_back(r);
            if (r->getStop()) 
            {
                stop[r->getStop()].push_back(r);
            }
        } catch (Error &e ) {
            saveDelete(r);
            // if only one rule return error
            if (bids->size() == 1) {
                throw e;
            }
            // FIXME else return number of successively installed bids
        }
      
    }
    
#ifdef DEBUG    
    log->dlog(ch, "Start all bids - it is going to activate them");
#endif      

    // group rules with same start time
    for (iter2 = start.begin(); iter2 != start.end(); iter2++) {
        e->addEvent(new ActivateBidsEvent(iter2->first-now, iter2->second));
    }
    
    // group rules with same stop time
    for (iter2 = stop.begin(); iter2 != stop.end(); iter2++) {
        e->addEvent(new RemoveBidsEvent(iter2->first-now, iter2->second));
    }

#ifdef DEBUG    
    log->dlog(ch, "Finished adding bids");
#endif      

}


/* -------------------- addBid -------------------- */

void BidManager::addBid(Bid *r)
{
  
#ifdef DEBUG    
    log->dlog(ch, "adding new bid with name = '%s'",
              r->getBidName().c_str());
#endif  

    // test for presence of bidSource/bidName combination
    // in bidDatabase in particular set
    if (getBid(r->getSetName(), r->getBidName())) {
        log->elog(ch, "bid %s.%s already installed",
                  r->getSetName().c_str(), r->getBidName().c_str());
        throw Error(408, "Bid with this name is already installed");
    }

    try {
        // could do some more checks here
        r->setState(RS_VALID);

#ifdef DEBUG    
    log->dlog(ch, "Bid Id = '%d'",
              r->getUId());
#endif 

        // resize vector if necessary
        if ((unsigned int)r->getUId() >= bidDB.size()) {
            bidDB.reserve(r->getUId() * 2 + 1);
            bidDB.resize(r->getUId() + 1);
        }

        // insert bid
        bidDB[r->getUId()] = r; 	

        // add new entry in index
        bidSetIndex[r->getSetName()][r->getBidName()] = r->getUId();
	
        tasks++;

#ifdef DEBUG    
    log->dlog(ch, "finish adding new bid with name = '%s'",
              r->getBidName().c_str());
#endif  

    } catch (Error &e) { 

        // adding new bid failed in some component
        // something failed -> remove bid from database
        delBid(r->getSetName(), r->getBidName(), NULL);
	
        throw e;
    }
}

void BidManager::activateBids(bidDB_t *bids, EventScheduler *e)
{
    bidDBIter_t             iter;

    for (iter = bids->begin(); iter != bids->end(); iter++) {
        Bid *r = (*iter);
        log->dlog(ch, "activate bid with name = '%s'", r->getBidName().c_str());
        r->setState(RS_ACTIVE);
	 
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
    ruleSetIndexIter_t r;

    r = ruleSetIndex.find(sname);

    if (r != ruleSetIndex.end()) {
        for (ruleIndexIter_t i = r->second.begin(); i != r->second.end(); i++) {
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
    if (bidSetIndex.find(sname) != bidSetIndex.end()) {
        for (bidSetIndexIter_t i = bidSetIndex.begin(); i != bidSetIndex.end(); i++) {
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

    tasks--;
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
    
    r->setState(RS_DONE);
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
