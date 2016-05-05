
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

#include "config.h"
#include "ParserFcts.h"
#include "BiddingObjectManager.h"
#include "Constants.h"
#include <pqxx/pqxx>

using namespace auction;

/* ------------------------- BiddingObjectManager ------------------------- */

BiddingObjectManager::BiddingObjectManager( int domain, string fdname, string fvname, string connectionDB) 
    : AuctioningObjectManager(domain, fdname, fvname, "BiddingObjectManager"), connectionDBStr(connectionDB)
{
        
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif


}


/* ------------------------- ~BiddingObjectManager ------------------------- */

BiddingObjectManager::~BiddingObjectManager()
{

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

}


/* ----------------------------- parseBidingObjects --------------------------- */

auctioningObjectDB_t *
BiddingObjectManager::parseBiddingObjects(string fname)
{

#ifdef DEBUG
    log->dlog(ch,"parseBiddingObjects");
#endif

    auctioningObjectDB_t *newBiddingObjects = new auctioningObjectDB_t();

    try {	
	
        BiddingObjectFileParser rfp = BiddingObjectFileParser(getDomain(), fname);
        
        rfp.parse(FieldDefManager::getFieldDefs(), FieldDefManager::getFieldVals(), newBiddingObjects );

#ifdef DEBUG
    log->dlog(ch, "biddingObjects parsed");
#endif

        return newBiddingObjects;

    } catch (Error &e) {

        for(auctioningObjectDBIter_t i= newBiddingObjects->begin(); i != newBiddingObjects->end(); i++) {
           saveDelete(*i);
        }
        saveDelete(newBiddingObjects);        
		log->dlog(ch, "Error: %s", e.getError().c_str());
        throw e;
    }
}


/* -------------------- parseBiddingObjectsBuffer -------------------- */

auctioningObjectDB_t *
BiddingObjectManager::parseBiddingObjectsBuffer(char *buf, int len)
{
    auctioningObjectDB_t *newBiddingObjects = new auctioningObjectDB_t();

    try {
			
        BiddingObjectFileParser rfp = BiddingObjectFileParser(getDomain(), buf, len);
        rfp.parse(FieldDefManager::getFieldDefs(), FieldDefManager::getFieldVals(), newBiddingObjects );

        return newBiddingObjects;
	
    } catch (Error &e) {

        for(auctioningObjectDBIter_t i=newBiddingObjects->begin(); i != newBiddingObjects->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(newBiddingObjects);
		log->dlog(ch, "Error: %s", e.getError().c_str());
        throw e;
    }
}


/* -------------------- parseMessage -------------------- */

auctioningObjectDB_t *
BiddingObjectManager::parseMessage(ipap_message *messageIn, ipap_template_container *templates)
{
    auctioningObjectDB_t *newBiddingObjects = new auctioningObjectDB_t();

    try {
			
        MAPIBiddingObjectParser rfp = MAPIBiddingObjectParser(getDomain());
        
        rfp.parse(FieldDefManager::getFieldDefs(), 
					FieldDefManager::getFieldVals(), 
						messageIn, newBiddingObjects, templates);

        return newBiddingObjects;
	
    } catch (Error &e) {

        for(auctioningObjectDBIter_t i=newBiddingObjects->begin(); i != newBiddingObjects->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(newBiddingObjects);
		log->dlog(ch, "Error: %s", e.getError().c_str());
        throw e;
    }
}


/* ---------------------------------- addAuctioningObjects ----------------------------- */

void BiddingObjectManager::addAuctioningObjects(auctioningObjectDB_t * _biddingObjects, EventScheduler *e)
{
    auctioningObjectDBIter_t        iter;
    
    biddingObjectTimeIndex_t     start;
    biddingObjectTimeIndex_t     stop;
    biddingObjectTimeIndexIter_t iter2;
    time_t              now = time(NULL);
    
    // add bids
    for (iter = _biddingObjects->begin(); iter != _biddingObjects->end();) 
    {
        AuctioningObject *a = *iter;
        BiddingObject *b = dynamic_cast<BiddingObject *>(a);
        
        try {
			biddingObjectIntervalList_t intervalList;
            b->calculateIntervals(now,  &intervalList);
            
            addBiddingObject(b);
						
            biddingObjectIntervalListIter_t intervIter;
            for ( intervIter = intervalList.begin(); intervIter != intervalList.end(); ++intervIter ){ 
				start[(intervIter->second).start].push_back(b);
				if ((intervIter->second).stop) 
				{
					stop[(intervIter->second).stop].push_back(b);
				}
			}

			// continue with the next Bidding object.
			iter++;
			
        } catch (Error &e ) {
            // skip the bidding object, as it has problems.
            _biddingObjects->erase(iter);
            saveDelete(b);
            
            // if only one rule return error
            if (_biddingObjects->size() == 1) {
				log->dlog(ch, "Error: %s", e.getError().c_str());
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
    log->dlog(ch, "adding new BiddingObject with name = %s.%s",
              b->getSet().c_str(), b->getName().c_str());
#endif  
				  
			  
    try {
		
		AuctioningObjectManager::addAuctioningObject(b);

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
        
#ifdef DEBUG    
    log->dlog(ch, "finish adding new BiddingObject with name = %s.%s",
						b->getSet().c_str(), b->getName().c_str() );
#endif  

    } catch (Error &e) { 

        // adding new BiddingObject failed in some component
        // something failed -> remove BiddingObject from database
        delBiddingObject(b, NULL);
		log->dlog(ch, "Error: %s", e.getError().c_str());	
        throw e;
    }
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


/* ------------------------- delBiddingObject ------------------------- */

void BiddingObjectManager::delBiddingObject(BiddingObject *r, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "removing BiddingObject with name = %s.%s", 
						r->getSet().c_str(), r->getName().c_str());
#endif

	assert( r != NULL );
	
    AuctioningObjectManager::delAuctioningObject(r);
    
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
	    
    if (e != NULL) {
        e->delBiddingObjectEvents(r->getUId());
    }

}


void BiddingObjectManager::delBiddingObject(string sname, string rname, EventScheduler *e)
{

	BiddingObject *b = dynamic_cast<BiddingObject *>(getAuctioningObject(sname, rname));
	
	delBiddingObject(b, e);
}

void BiddingObjectManager::delBiddingObject(int uid, EventScheduler *e)
{
	BiddingObject *b = dynamic_cast<BiddingObject *>(getAuctioningObject(uid));
	
	delBiddingObject(b, e);
}


void BiddingObjectManager::delBiddingObjects(string sname, EventScheduler *e)
{
	auctioningObjectIndex_t *objects = getAuctioningObjects(sname);
	auctioningObjectIndexIter_t iter;
	
    for (auctioningObjectIndexIter_t i = objects->begin(); i != objects->end(); i++) {						
        BiddingObject *o = dynamic_cast<BiddingObject *>(getAuctioningObject(sname, i->first));
        delBiddingObject(o,e);
    }
}

void 
BiddingObjectManager::delAuctioningObjects(auctioningObjectDB_t *biddingObjects, EventScheduler *e)
{

    auctioningObjectDBIter_t iter;

    for (iter = biddingObjects->begin(); iter != biddingObjects->end(); iter++) {
		AuctioningObject *ao = *iter;
		BiddingObject *o = dynamic_cast<BiddingObject *>(ao);
        delBiddingObject(o, e);
    }

}


/* -------------------- storeBiddingObjectAsDone -------------------- */

void BiddingObjectManager::storeBiddingObjectAsDone(BiddingObject *r)
{

#ifdef DEBUG    
    log->dlog(ch, "StoreBiddingObjects name = %s.%s", r->getSet().c_str(), 
					r->getName().c_str());
#endif
    
    
    if (connectionDBStr.empty())
    {
		AuctioningObjectManager::storeAuctioningObjectAsDone(r);
	} 
	else 
	{

#ifdef DEBUG    
		log->dlog(ch, "connection Str = '%s'", connectionDBStr.c_str());
#endif
		
		try
		{
			
			r->setState(AO_DONE);
			
			// Store in the database
			pqxx::connection c(connectionDBStr);

#ifdef DEBUG    
			log->dlog(ch, "database connected");
#endif

#ifdef HAVE_PQXX40
			r->save_ver4(c);
#else
			r->save_ver3(c);
#endif
		
			// release id
			idSource.freeId(r->getUId());

			// remove rule
			saveDelete(r);
		} catch(const std::exception &e){
			throw Error("Error connecting to the database %s", e.what());
		}

	}
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


/* ------------------------- getInfo ------------------------- */

string BiddingObjectManager::getInfo(string sname, string rname)
{

    string info;
    AuctioningObject *ao;
  
    ao = getAuctioningObject(sname, rname);
    BiddingObject *b = dynamic_cast<BiddingObject *>(ao);

    if ( b == NULL )
    {
        // check done tasks
        AuctioningObject *ao = getAuctioningObjectDone(sname, rname);
        b = dynamic_cast<BiddingObject *>(ao);
        if (b != NULL){
			info = b->getInfo();
        } else {
            throw Error("no auctioning object with bid name '%s.%s'", sname.c_str(), rname.c_str());
        }
    } else {
        // auction object with given identification is in database
        info = b->getInfo();
    }
    
    return info;
}

/* ------------------------- getInfo ------------------------- */

string BiddingObjectManager::getInfo(int uid)
{ 
	AuctioningObject *ao = getAuctioningObject(uid); 
	
	BiddingObject *b = dynamic_cast<BiddingObject *>(ao);
	
	if (b != NULL)
		return b->getInfo();
	else
		return string();
}

/* ------------------------- getInfo ------------------------- */

string BiddingObjectManager::getInfo(string sname)
{
    ostringstream s;

	auctioningObjectIndex_t *objects = getAuctioningObjects(sname);
	auctioningObjectIndexIter_t iter;

    for (auctioningObjectIndexIter_t i = objects->begin(); i != objects->end(); i++) {						
        BiddingObject *o = dynamic_cast<BiddingObject *>(getAuctioningObject(sname, i->first));
        s << o->getInfo();
    }
    
    return s.str();
}

/* ------------------------- getInfo ------------------------- */

string BiddingObjectManager::getInfo()
{
    ostringstream s;
    auctioningObjectDBIter_t iter;

    for (iter = getAuctioningObjects().begin(); iter != getAuctioningObjects().end(); iter++) 
    {
        BiddingObject *bo = dynamic_cast<BiddingObject *>(*iter);
        s << bo->getInfo();
    }
    
    return s.str();
}

/* ------------------------- dump ------------------------- */

void BiddingObjectManager::dump( ostream &os )
{
    
    auctioningObjectDBIter_t iter;
    os << "Bidding Manager dump :" << endl;
    
    auctioningObjectDB_t object = getAuctioningObjects();
    for (iter = object.begin(); iter != object.end(); ++iter )
    {
		AuctioningObject *ao = *iter;
		BiddingObject *a = dynamic_cast<BiddingObject *>(ao);
		os << a->getInfo() << endl;
    }
}

/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, BiddingObjectManager &am )
{
    am.dump(os);
    return os;
}
