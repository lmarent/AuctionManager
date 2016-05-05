
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

#include "ParserFcts.h"
#include "AuctionManager.h"
#include "Constants.h"

using namespace auction;

/* ------------------------- AuctionManager ------------------------- */

AuctionManager::AuctionManager( int domain, string fdname, string fvname, bool _immediateStart) 
    : AuctioningObjectManager(domain, fdname, fvname, "AuctionManager"), immediateStart(_immediateStart)
{

#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif
	
}


/* ------------------------- ~AuctionManager ------------------------- */

AuctionManager::~AuctionManager()
{

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif
	
}

bool 
AuctionManager::getImmediateStart()
{
	return immediateStart;
}

Auction * 
AuctionManager::getAuction(string sname, string rname)
{
	
	AuctioningObject *ao = getAuctioningObject(sname,rname);
	if (ao != NULL)
		return dynamic_cast<Auction *>(ao);
	else
		return NULL;
}

/* ----------------------------- parseAuctions --------------------------- */

auctioningObjectDB_t *
AuctionManager::parseAuctions(string fname, ipap_template_container *templates)
{

#ifdef DEBUG
    log->dlog(ch,"ParseAuctions");
#endif

    auctioningObjectDB_t *new_auctions = new auctioningObjectDB_t();

    try {	
	
        AuctionFileParser afp = AuctionFileParser(getDomain(), fname);

        afp.parse(FieldDefManager::getFieldDefs(), new_auctions, templates);

#ifdef DEBUG
    log->dlog(ch, "Auctions parsed");
#endif

        return new_auctions;

    } catch (Error &e) {

        for(auctioningObjectDBIter_t i=new_auctions->begin(); i != new_auctions->end(); i++) {
           saveDelete(*i);
        }
        saveDelete(new_auctions);
        throw e;
    }
}


/* -------------------- parseBidsBuffer -------------------- */

auctioningObjectDB_t *
AuctionManager::parseAuctionsBuffer(char *buf, int len, 
									ipap_template_container *templates)
{
    auctioningObjectDB_t *new_auctions = new auctioningObjectDB_t();

    try {
			
        AuctionFileParser afp = AuctionFileParser(getDomain(), buf, len);
        
        afp.parse(FieldDefManager::getFieldDefs(), new_auctions, templates);

        return new_auctions;
	
    } catch (Error &e) {

        for(auctioningObjectDBIter_t i = new_auctions->begin(); i != new_auctions->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_auctions);
        throw e;
    }
}


auctioningObjectDB_t *
AuctionManager::parseMessage(ipap_message *messageIn, ipap_template_container *templates)
{
    auctioningObjectDB_t *new_auctions = new auctioningObjectDB_t();

    try {
			       
       	MAPIAuctionParser mpap = MAPIAuctionParser(getDomain());
        
        mpap.parse(FieldDefManager::getFieldDefs(), messageIn, new_auctions, templates);
		
        return new_auctions;
	
    } catch (Error &e) {

        for(auctioningObjectDBIter_t i=new_auctions->begin(); i != new_auctions->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_auctions);
        throw e;
    }
}


/* ---------------------------------- addAuctions ----------------------------- */

void AuctionManager::addAuctioningObjects(auctioningObjectDB_t * _auctions, EventScheduler *e)
{
    auctioningObjectDBIter_t    iter;
    auctionTimeIndex_t     		start;
    auctionTimeIndex_t     		startnow;
    auctionTimeIndex_t     		stop;
    auctionTimeIndexIter_t 		iter2;
    time_t            			now = time(NULL);
    
    // add auctions
    for (iter = _auctions->begin(); iter != _auctions->end(); iter++) 
    {
        
        AuctioningObject *ao = (*iter);
        Auction *a = dynamic_cast<Auction *>(ao);
    
        try {
            
            addAuctioningObject(a);

			if (getImmediateStart()){
				startnow[now].push_back(a);
			} else {
				start[a->getStart()].push_back(a);
			}
			
            if (a->getStop()) 
            {
                stop[a->getStop()].push_back(a);
            }

        } catch (Error &e ) {
            saveDelete(a);
            if (e.getErrorNo() != 408) {
				// only throws the error, if its is different from duplicate key.
                throw e;
            }
        }
    }
    
//#ifdef DEBUG    
    log->log(ch, "Start all auctions - it is going to activate them, events %d", e->getNbrEvents());
//#endif      
	
	if (getImmediateStart()){ 
		// createProcessRequests for the auctions
		for (iter2 = startnow.begin(); iter2 != startnow.end(); iter2++) {
			e->addEvent(new ActivateAuctionsEvent(iter2->first-now, iter2->second));
		}
	} else {
		// group auctions with same start time
		for (iter2 = start.begin(); iter2 != start.end(); iter2++) {
			e->addEvent(new ActivateAuctionsEvent(iter2->first-now, iter2->second));
		}
	}
	
    // group auctions with same stop time
    for (iter2 = stop.begin(); iter2 != stop.end(); iter2++) {
        e->addEvent(new RemoveAuctionsEvent(iter2->first-now, iter2->second));
    }

//#ifdef DEBUG    
    log->log(ch, "Finished adding auctions, events: %d", e->getNbrEvents());
//#endif      

}


/* ---------------------- get_ipap_message ------------------------- */
ipap_message * 
AuctionManager::get_ipap_message(auctioningObjectDB_t *auctions, 
 								 ipap_template_container *templates,
								 bool useIPV6, string sAddressIPV4, 
								 string sAddressIPV6, uint16_t port)
{

	MAPIAuctionParser mpap = MAPIAuctionParser(getDomain());

	return mpap.get_ipap_message(FieldDefManager::getFieldDefs(), auctions,templates, 
								 useIPV6, sAddressIPV4, sAddressIPV6, port);
}



/* ------------------------- delAuction ------------------------- */

void AuctionManager::delAuction(Auction *a, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "removing auction with name = %s.%s", 
						a->getSet().c_str(), a->getName().c_str());
#endif
	
	AuctioningObjectManager::delAuctioningObject(a);
	
    if (e != NULL) {
        e->delAuctionEvents(a->getUId());
    }

}

void AuctionManager::delAuction(int uid, EventScheduler *e)
{
	Auction *a = dynamic_cast<Auction *>(getAuctioningObject(uid));
	
	if (a != NULL)
		delAuction(a, e);
}



void AuctionManager::delAuction(string sname, string rname, EventScheduler *e)
{

	Auction *a = dynamic_cast<Auction *>(getAuctioningObject(sname, rname));
	
	if (a != NULL)
		delAuction(a, e);
}

void 
AuctionManager::delAuctions(string sname, EventScheduler *e)
{
	auctioningObjectIndex_t *objects = getAuctioningObjects(sname);
	auctioningObjectIndexIter_t iter;
	
    for (auctioningObjectIndexIter_t i = objects->begin(); i != objects->end(); i++) 
    {						
        Auction *o = dynamic_cast<Auction *>(getAuctioningObject(sname, i->first));
        delAuction(o,e);
    }
}



void 
AuctionManager::delAuctioningObjects(auctioningObjectDB_t *auctions, EventScheduler *e)
{

    auctioningObjectDBIter_t iter;

    for (iter = auctions->begin(); iter != auctions->end(); iter++) {
        AuctioningObject * ao = *iter;
        Auction *a = dynamic_cast<Auction *>(ao); 
        delAuction(a, e);
    }

}

/* ------------------------- dump ------------------------- */

void AuctionManager::dump( ostream &os )
{
    
    auctioningObjectDBIter_t iter;
    os << "Auction Manager dump :" << endl;
    
    auctioningObjectDB_t object = getAuctioningObjects();
    for (iter = object.begin(); iter != object.end(); ++iter ){
		AuctioningObject *ao = *iter;
		Auction *a = dynamic_cast<Auction *>(ao);
		os << a->getInfo() << endl;
    }
}

/* ------------------------- operator<< ------------------------- */

ostream& auction::operator<< ( ostream &os, AuctionManager &am )
{
    am.dump(os);
    return os;
}

void AuctionManager::getIds(auctioningObjectDB_t *_auctions, auctionSet_t &setParam)
{
    
    auctioningObjectDBIter_t  iter;
    for (iter = _auctions->begin(); iter != _auctions->end(); iter++) 
    {
        Auction *a = dynamic_cast<Auction *>(*iter);
        Auction *a2 = dynamic_cast<Auction *>(getAuctioningObject(a->getSet(), a->getName()));
        if (a2== NULL){
#ifdef DEBUG    
			log->dlog(ch, "GetId not found for auction %s.%s", 
						a->getSet().c_str(), a->getName().c_str());
#endif
			
		} else {
			setParam.insert(a2->getUId());
		}
    }
}

void AuctionManager::incrementReferences(auctionSet_t & setParam, string sessionId)
{
	auctionSetIter_t iter;
	for (iter = setParam.begin(); iter != setParam.end(); ++iter)
	{
		int _uid = *iter;
		Auction *a = dynamic_cast<Auction *>(getAuctioningObject(_uid));
		a->incrementSessionReferences(sessionId);
	}
}

void AuctionManager::decrementReferences(auctionSet_t & setParam, string sessionId)
{
	auctionSetIter_t iter;
	for (iter = setParam.begin(); iter != setParam.end(); ++iter)
	{
		int _uid = *iter;
		Auction *a = dynamic_cast<Auction *>(getAuctioningObject(_uid));
		
		// A remove to the auction could be done before, so we have to find the auction in stored auctions. 
		if (a == NULL)
			a = dynamic_cast<Auction *>(getAuctioningObjectDone(_uid));
		
		if (a !=NULL)
			a->decrementSessionReferences(sessionId);
	}
}


/* ------------------------- getInfo ------------------------- */

string AuctionManager::getInfo(string sname, string rname)
{

    string info;
    AuctioningObject *ao;
  
    ao = getAuctioningObject(sname, rname);
    Auction *a = dynamic_cast<Auction *>(ao);

    if (a == NULL) 
    {
        // check done tasks
        AuctioningObject *ao = getAuctioningObjectDone(sname, rname);
        a = dynamic_cast<Auction *>(ao);
        if (a != NULL){
			info = a->getInfo();
        } else {
            throw Error("no auctioning object with bid name '%s.%s'", sname.c_str(), rname.c_str());
        }
    } else {
        // auction object with given identification is in database
        info = a->getInfo();
    }
    
    return info;
}

/* ------------------------- getInfo ------------------------- */

string AuctionManager::getInfo(int uid)
{ 
	AuctioningObject *ao = getAuctioningObject(uid); 
	
	Auction *a = dynamic_cast<Auction *>(ao);
	
	if (a != NULL)
		return a->getInfo();
	else
		return string();
}

/* ------------------------- getInfo ------------------------- */

string AuctionManager::getInfo(string sname)
{
    ostringstream s;

	auctioningObjectIndex_t *objects = getAuctioningObjects(sname);
	auctioningObjectIndexIter_t iter;

    for (auctioningObjectIndexIter_t i = objects->begin(); i != objects->end(); i++) {						
        Auction *o = dynamic_cast<Auction *>(getAuctioningObject(sname, i->first));
        s << o->getInfo();
    }
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string AuctionManager::getInfo()
{
    ostringstream s;
    auctioningObjectDBIter_t iter;

    for (iter = getAuctioningObjects().begin(); iter != getAuctioningObjects().end(); iter++) 
    {
        Auction *auction = dynamic_cast<Auction *>(*iter);
        s << auction->getInfo();
    }
    
    return s.str();
}
