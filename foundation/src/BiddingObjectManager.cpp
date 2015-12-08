
/*! \file   BiddingObjectManager.cpp

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
	BiddingObject database
    Code based on Netmate Implementation

    $Id: BiddingObjectManager.h 748 2015-07-23 14:00:00Z amarentes $

*/

#include "ParserFcts.h"
#include "BiddingObjectManager.h"
#include "Constants.h"
#include <pqxx/pqxx>

using namespace auction;

/* ------------------------- BiddingObjectManager ------------------------- */

BiddingObjectManager::BiddingObjectManager( int domain, string fdname, string fvname, string connectionDB) 
    : FieldDefManager(fdname, fvname), biddingObjects(0), idSource(1), domain(domain), connectionDBStr(connectionDB)
{
    log = Logger::getInstance();
    ch = log->createChannel("BiddingObjectManager");
        
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif


}


/* ------------------------- ~BiddingObjectManager ------------------------- */

BiddingObjectManager::~BiddingObjectManager()
{
    biddingObjectDBIter_t iter;

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

    for (iter = biddingObjectDB.begin(); iter != biddingObjectDB.end(); iter++) {
        if (*iter != NULL) {
            // delete rule
            saveDelete(*iter);
        } 
    }

    for (biddingObjectDoneIter_t i = biddingObjectDone.begin(); i != biddingObjectDone.end(); i++) {
        saveDelete(*i);
    }

#ifdef DEBUG
    log->dlog(ch,"Finish Shutdown");
#endif    
}



/* -------------------------- getRule ----------------------------- */

BiddingObject *
BiddingObjectManager::getBiddingObject(int uid)
{
    if ((uid >= 0) && ((unsigned int)uid <= biddingObjectDB.size())) {
        return biddingObjectDB[uid];
    } else {
        return NULL;
    }
}



/* -------------------- getBiddingObject -------------------- */

BiddingObject *
BiddingObjectManager::getBiddingObject(string sname, string rname)
{
    biddingObjectSetIndexIter_t iter;
    biddingObjectIndexIter_t iter2;

    iter = biddingObjectSetIndex.find(sname);
    if (iter != biddingObjectSetIndex.end()) {		
        iter2 = iter->second.find(rname);
        if (iter2 != iter->second.end()) {
            return getBiddingObject(iter2->second);
        }
        else
        {
#ifdef DEBUG
    log->dlog(ch,"BiddingObject Id not found %s.%s",sname.c_str(), rname.c_str());
#endif		
			
		}
    }
    else
    {
#ifdef DEBUG
    log->dlog(ch,"Bidding Object set not found");
#endif		
	}

    return NULL;
}


/* -------------------- getBiddingObjects -------------------- */

biddingObjectIndex_t *
BiddingObjectManager::getBiddingObjects(string sname)
{
    biddingObjectSetIndexIter_t iter;

    iter = biddingObjectSetIndex.find(sname);
    if (iter != biddingObjectSetIndex.end()) {
        return &(iter->second);
    }

    return NULL;
}

/* -------------------- getBiddingObjects -------------------- */
vector<int> 
BiddingObjectManager::getBiddingObjects(string aset, string aname)
{

    auctionSetBidIndexIter_t iter;

    iter = bidAuctionSetIndex.find(aset);
    if (iter != bidAuctionSetIndex.end()) {
        auctionBidIndexIter_t auctionBidIndexIter = (iter->second).find(aname);
        if (auctionBidIndexIter != (iter->second).end()){
			return auctionBidIndexIter->second;
		}
    }
	
	vector<int> list_return;
    return list_return;
	 
}


biddingObjectDB_t 
BiddingObjectManager::getBiddingObjects()
{
    biddingObjectDB_t ret;

    for (biddingObjectSetIndexIter_t r = biddingObjectSetIndex.begin(); r != biddingObjectSetIndex.end(); r++) {
        for (biddingObjectIndexIter_t i = r->second.begin(); i != r->second.end(); i++) {
            ret.push_back(getBiddingObject(i->second));
        }
    }

    return ret;
}

/* ----------------------------- parseBidingObjects --------------------------- */

biddingObjectDB_t *
BiddingObjectManager::parseBiddingObjects(string fname)
{

#ifdef DEBUG
    log->dlog(ch,"parseBiddingObjects");
#endif

    biddingObjectDB_t *newBiddingObjects = new biddingObjectDB_t();

    try {	
	
        BiddingObjectFileParser rfp = BiddingObjectFileParser(getDomain(), fname);
        
        rfp.parse(FieldDefManager::getFieldDefs(), FieldDefManager::getFieldVals(), newBiddingObjects );

#ifdef DEBUG
    log->dlog(ch, "biddingObjects parsed");
#endif

        return newBiddingObjects;

    } catch (Error &e) {

        for(biddingObjectDBIter_t i=newBiddingObjects->begin(); i != newBiddingObjects->end(); i++) {
           saveDelete(*i);
        }
        saveDelete(newBiddingObjects);
        throw e;
    }
}


/* -------------------- parseBiddingObjectsBuffer -------------------- */

biddingObjectDB_t *
BiddingObjectManager::parseBiddingObjectsBuffer(char *buf, int len)
{
    biddingObjectDB_t *newBiddingObjects = new biddingObjectDB_t();

    try {
			
        BiddingObjectFileParser rfp = BiddingObjectFileParser(getDomain(), buf, len);
        rfp.parse(FieldDefManager::getFieldDefs(), FieldDefManager::getFieldVals(), newBiddingObjects );

        return newBiddingObjects;
	
    } catch (Error &e) {

        for(biddingObjectDBIter_t i=newBiddingObjects->begin(); i != newBiddingObjects->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(newBiddingObjects);
        throw e;
    }
}


/* -------------------- parseMessage -------------------- */

biddingObjectDB_t *
BiddingObjectManager::parseMessage(ipap_message *messageIn, ipap_template_container *templates)
{
    biddingObjectDB_t *newBiddingObjects = new biddingObjectDB_t();

    try {
			
        MAPIBiddingObjectParser rfp = MAPIBiddingObjectParser(getDomain());
        
        rfp.parse(FieldDefManager::getFieldDefs(), 
					FieldDefManager::getFieldVals(), 
						messageIn, newBiddingObjects, templates);

        return newBiddingObjects;
	
    } catch (Error &e) {

        for(biddingObjectDBIter_t i=newBiddingObjects->begin(); i != newBiddingObjects->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(newBiddingObjects);
        throw e;
    }
}


/* ---------------------------------- addBiddingObjects ----------------------------- */

void BiddingObjectManager::addBiddingObjects(biddingObjectDB_t * _biddingObjects, EventScheduler *e)
{
    biddingObjectDBIter_t        iter;
    biddingObjectTimeIndex_t     start;
    biddingObjectTimeIndex_t     stop;
    biddingObjectTimeIndexIter_t iter2;
    time_t              now = time(NULL);
    
    // add bids
    for (iter = _biddingObjects->begin(); iter != _biddingObjects->end(); iter++) {
        BiddingObject *b = (*iter);
        
        try {
			biddingObjectIntervalList_t intervalList;
            addBiddingObject(b);
			b->calculateIntervals(now,  &intervalList);
			
            biddingObjectIntervalListIter_t intervIter;
            for ( intervIter = intervalList.begin(); intervIter != intervalList.end(); ++intervIter ){ 
				start[(intervIter->second).start].push_back(b);
				if ((intervIter->second).stop) 
				{
					stop[(intervIter->second).stop].push_back(b);
				}
			}
        } catch (Error &e ) {
            saveDelete(b);
            // if only one rule return error
            if (_biddingObjects->size() == 1) {
                throw e;
            }
            // FIXME else return number of successively installed bidding objects
        }
      
    }
    
#ifdef DEBUG    
    log->dlog(ch, "Start all bidding objects - it is going to activate them");
#endif      

    // group bidding objects with same start time
    for (iter2 = start.begin(); iter2 != start.end(); iter2++) {
		e->addEvent( new ActivateBiddingObjectsEvent(iter2->first-now, iter2->second));
    }
    
    // group rules with same stop time
    for (iter2 = stop.begin(); iter2 != stop.end(); iter2++) {
		// Iterates over the auctions configured for the BiddingObject.
		e->addEvent( new RemoveBiddingObjectsEvent(iter2->first-now, iter2->second));
    }

#ifdef DEBUG    
    log->dlog(ch, "Finished adding bidding objects");
#endif      

}


/* -------------------- addBiddingObject -------------------- */

void BiddingObjectManager::addBiddingObject(BiddingObject *b)
{
  
#ifdef DEBUG    
    log->dlog(ch, "adding new BiddingObject with name = '%s'",
              b->getBiddingObjectName().c_str());
#endif  
				  
			  
    // test for presence of bidSource/bidName combination
    // in bidDatabase in particular set
    if (getBiddingObject(b->getBiddingObjectSet(), b->getBiddingObjectName())) {
        log->elog(ch, "BiddingObject %s.%s already installed",
                  b->getBiddingObjectSet().c_str(), b->getBiddingObjectName().c_str());
        throw Error(408, "BiddingObject with this name is already installed");
    }

    try {

		// Assigns the new Id.
		b->setUId(idSource.newId());

        // could do some more checks here
        b->setState(AO_VALID);

#ifdef DEBUG    
		log->dlog(ch, "BiddingObject Id = '%d'", b->getUId());
#endif 

        // resize vector if necessary
        if ((unsigned int)b->getUId() >= biddingObjectDB.size()) {
            biddingObjectDB.reserve(b->getUId() * 2 + 1);
            biddingObjectDB.resize(b->getUId() + 1);
        }

        // insert BiddingObject
        biddingObjectDB[b->getUId()] = b; 	

        // add new entry in index
        biddingObjectSetIndex[b->getBiddingObjectSet()][b->getBiddingObjectName()] = b->getUId();


		string aSet = b->getAuctionSet();
		string aName = b->getAuctionName();
		
		auctionSetBidIndexIter_t setIter;
		setIter = bidAuctionSetIndex.find(aSet);
		if (setIter != bidAuctionSetIndex.end())
		{
			auctionBidIndexIter_t actBidIter = (setIter->second).find(aName);
			if (actBidIter != (setIter->second).end()){
				(actBidIter->second).push_back(b->getUId());
			}
			else{
				vector<int> listBids;
				listBids.push_back(b->getUId());
				bidAuctionSetIndex[aSet][aName] = listBids;
			}
		} else {
			vector<int> listBids;
			listBids.push_back(b->getUId());
			bidAuctionSetIndex[aSet][aName] = listBids;
		}
        
        biddingObjects++;

#ifdef DEBUG    
    log->dlog(ch, "finish adding new BiddingObject with name = '%s'",
              b->getBiddingObjectName().c_str());
#endif  

    } catch (Error &e) { 

        // adding new BiddingObject failed in some component
        // something failed -> remove BiddingObject from database
        delBiddingObject(b->getBiddingObjectSet(), b->getBiddingObjectName(), NULL);
	
        throw e;
    }
}

void 
BiddingObjectManager::activateBiddingObjects(biddingObjectDB_t *biddingObjects)
{
    biddingObjectDBIter_t             iter;

    for (iter = biddingObjects->begin(); iter != biddingObjects->end(); iter++) {
        BiddingObject *b = (*iter);
        log->dlog(ch, "activate BiddingObject with name = '%s'", b->getBiddingObjectName().c_str());
        b->setState(AO_ACTIVE);
    }
}


/* ------------------------- getInfo ------------------------- */

string BiddingObjectManager::getInfo(string sname, string rname)
{
    ostringstream s;
    string info;
    BiddingObject *r;
  
    r = getBiddingObject(sname, rname);

    if (r == NULL) {
        // check done tasks
        for (biddingObjectDoneIter_t i = biddingObjectDone.begin(); i != biddingObjectDone.end(); i++) {
            if (((*i)->getBiddingObjectName() == rname) && ((*i)->getBiddingObjectSet() == sname)) {
                info = (*i)->getInfo();
            }
        }
        
        if (info.empty()) {
            throw Error("no BiddingObject with BiddingObject name '%s.%s'", sname.c_str(), rname.c_str());
        }
    } else {
        // Bidding object with given identification is in database
        info = r->getInfo();
    }
    
    s << info;

    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string BiddingObjectManager::getInfo(string sname)
{
    ostringstream s;
    biddingObjectSetIndexIter_t b;

    b = biddingObjectSetIndex.find(sname);

    if (b != biddingObjectSetIndex.end()) {
        for (biddingObjectIndexIter_t i = b->second.begin(); i != b->second.end(); i++) {
            s << getInfo(sname, i->first);
        }
    } else {
        s << "No such Bidding Object set" << endl;
    }
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string BiddingObjectManager::getInfo()
{
    ostringstream s;
    biddingObjectSetIndexIter_t iter;

    for (iter = biddingObjectSetIndex.begin(); iter != biddingObjectSetIndex.end(); iter++) {
        s << getInfo(iter->first);
    }
    
    return s.str();
}


/* ------------------------- delBiddingObject ------------------------- */

void BiddingObjectManager::delBiddingObject(string sname, string rname, EventScheduler *e)
{
    BiddingObject *r;

#ifdef DEBUG    
    log->dlog(ch, "Deleting BiddingObject set= %s name = '%s'",
              sname.c_str(), rname.c_str());
#endif  


    if (sname.empty() && rname.empty()) {
        throw Error("incomplete rule set or name specified");
    }

    r = getBiddingObject(sname, rname);

    if (r != NULL) {
        delBiddingObject(r, e);
    } else {
        throw Error("BiddingObject %s.%s does not exist", sname.c_str(),rname.c_str());
    }
}


/* ------------------------- delBiddingObject ------------------------- */

void BiddingObjectManager::delBiddingObject(int uid, EventScheduler *e)
{
    BiddingObject *r;

    r = getBiddingObject(uid);

    if (r != NULL) {
        delBiddingObject(r, e);
    } else {
        throw Error("BiddingObject uid %d does not exist", uid);
    }
}


/* ------------------------- delBiddingObjects ------------------------- */

void BiddingObjectManager::delBiddingObjects(string sname, EventScheduler *e)
{
    
    if (biddingObjectSetIndex.find(sname) != biddingObjectSetIndex.end()) 
    {
		biddingObjectSetIndexIter_t iter = biddingObjectSetIndex.find(sname);
		biddingObjectIndex_t bidIndex = iter->second;
        for (biddingObjectIndexIter_t i = bidIndex.begin(); i != bidIndex.end(); i++) 
        {
            delBiddingObject(getBiddingObject(sname, i->first),e);
        }
    }
}


/* ------------------------- delBiddingObject ------------------------- */

void BiddingObjectManager::delBiddingObject(BiddingObject *r, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "removing BiddingObject with name = '%s'", r->getBiddingObjectName().c_str());
#endif

    // remove BiddingObject from database and from index
    biddingObjectDB[r->getUId()] = NULL;
    biddingObjectSetIndex[r->getBiddingObjectSet()].erase(r->getBiddingObjectName());
    
    // Find the corresponding nodes in the auction BiddingObject index and deletes
	vector<int>::iterator actBidIter;
	auctionSetBidIndexIter_t setNode;  
	auctionBidIndexIter_t auctionNode; 
	
	setNode = bidAuctionSetIndex.find(r->getAuctionSet());
	auctionNode = (setNode->second).find(r->getAuctionName());
		
	for (actBidIter = (auctionNode->second).begin(); 
			actBidIter != (auctionNode->second).end(); ++actBidIter){
					
		if (*actBidIter == r->getUId()){
			(auctionNode->second).erase(actBidIter);
			break;
		}
	}
		
	// delete the auction name if empty.
	auctionNode = (setNode->second).find(r->getAuctionName());
	if ((auctionNode->second).empty()){
		bidAuctionSetIndex[r->getAuctionSet()].erase(auctionNode);
	}
			
	// delete BiddingObject auction set if empty
	setNode = bidAuctionSetIndex.find(r->getAuctionSet());
	if ((setNode->second).empty()) {
		bidAuctionSetIndex.erase(setNode);
	}
	
    // delete BiddingObject set if empty
    if (biddingObjectSetIndex[r->getBiddingObjectSet()].empty()) {
        biddingObjectSetIndex.erase(r->getBiddingObjectSet());
    }
    
    if (e != NULL) {
        e->delBiddingObjectEvents(r->getUId());
    }

    // remove BiddingObject from database and from index
    storeBiddingObjectAsDone(r);

    biddingObjects--;
}


/* ------------------------- delBiddingObjects ------------------------- */

void BiddingObjectManager::delBiddingObjects(biddingObjectDB_t *bids, EventScheduler *e)
{
    biddingObjectDBIter_t iter;

    for (iter = bids->begin(); iter != bids->end(); iter++) {
        delBiddingObject(*iter, e);
    }
}


/* -------------------- storeBiddingObjectAsDone -------------------- */

void BiddingObjectManager::storeBiddingObjectAsDone(BiddingObject *r)
{

#ifdef DEBUG    
    log->dlog(ch, "StoreBiddingObjects name = '%s'", r->getBiddingObjectName().c_str());
#endif
    
    r->setState(AO_DONE);
    
    if (connectionDBStr.empty()){
    
		biddingObjectDone.push_back(r);

		if (biddingObjectDone.size() > DONE_LIST_SIZE) {
			// release id
			idSource.freeId(biddingObjectDone.front()->getUId());
			// remove rule
			saveDelete(biddingObjectDone.front());
			biddingObjectDone.pop_front();
		}
	} else {

#ifdef DEBUG    
		log->dlog(ch, "connection Str = '%s'", connectionDBStr.c_str());
#endif
		// Store in the database
		pqxx::connection c(connectionDBStr);
		r->save(c);

		// release id
		idSource.freeId(r->getUId());
		// remove rule
		saveDelete(r);

	}
}


/* ------------------------- dump ------------------------- */

void BiddingObjectManager::dump( ostream &os )
{
    
    os << "BiddingObjectManager dump :" << endl;
    os << getInfo() << endl;
    
}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, BiddingObjectManager &rm )
{
    rm.dump(os);
    return os;
}

/* ---------------------- get_ipap_message ------------------------- */
ipap_message * BiddingObjectManager::get_ipap_message(BiddingObject *biddingObject, 
													  Auction *auction,
													  ipap_template_container *templates)
{

	MAPIBiddingObjectParser mbop = MAPIBiddingObjectParser(getDomain());

	return mbop.get_ipap_message(FieldDefManager::getFieldDefs(), 
								 biddingObject, auction, templates );
}
