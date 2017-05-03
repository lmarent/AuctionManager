
/*! \file   ResourceManager.cpp

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

    $Id: ResourceManager.h 748 2016-05-02 13:50:00Z amarentes $

*/

#include "ParserFcts.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "Constants.h"

using namespace auction;

/* ------------------------- ResourceManager ------------------------- */

ResourceManager::ResourceManager( int domain, string fdname, string fvname) 
    : AuctioningObjectManager(domain, fdname, fvname, "ResourceManager")
{

#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif
	
}


/* ------------------------- ~ResourceManager ------------------------- */

ResourceManager::~ResourceManager()
{

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif
	
}


Resource * 
ResourceManager::getResource(string sname, string rname)
{
	
	AuctioningObject *ao = getAuctioningObject(sname,rname);
	if (ao != NULL)
		return dynamic_cast<Resource *>(ao);
	else
		return NULL;
}


/* ---------------------------------- addResources ----------------------------- */

void ResourceManager::addAuctioningObjects(auctioningObjectDB_t * _resources, EventScheduler *e)
{

#ifdef DEBUG    
    log->dlog(ch, "Starting Adding resources");
#endif 	
	
    auctioningObjectDBIter_t    iter;
    resourceTimeIndex_t     		start;
    resourceTimeIndex_t     		stop;
    resourceTimeIndexIter_t 		iter2;
    
    // add auctions
    for (iter = _resources->begin(); iter != _resources->end(); iter++) 
    {
        
        AuctioningObject *ao = (*iter);
        Resource *a = dynamic_cast<Resource *>(ao);
    
        try {
            
            addAuctioningObject(a);

            start[a->getStart()].push_back(a);
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
    
#ifdef DEBUG    
    log->dlog(ch, "Start all resources - it is going to activate them");
#endif      

    // group auctions with same start time
    //for (iter2 = start.begin(); iter2 != start.end(); iter2++) {
    //    e->addEvent(new ActivateResourcesEvent(iter2->first-now, iter2->second));
    //}
    
    // group auctions with same stop time
    //for (iter2 = stop.begin(); iter2 != stop.end(); iter2++) {
    //    e->addEvent(new RemoveResourcesEvent(iter2->first-now, iter2->second));
    //}

#ifdef DEBUG    
    log->dlog(ch, "Finished adding resources");
#endif      

}


/* ------------------------- delResource ------------------------- */

void ResourceManager::delResource(Resource *r, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "removing resource with name = %s.%s", 
						r->getSet().c_str(), r->getName().c_str());
#endif
	
	AuctioningObjectManager::delAuctioningObject(r);
	
    // TODO AM: Create resource event.
    //if (e != NULL) {
    //    e->delResourceEvents(a->getUId());
    //}

}

/* ------------------------- delResource ------------------------- */

void ResourceManager::delResource(int uid, EventScheduler *e)
{
	Resource *r = dynamic_cast<Resource *>(getAuctioningObject(uid));
	
	if (r != NULL)
		delResource(r, e);
}

/* ------------------------- delResource ------------------------- */

void ResourceManager::delResource(string sname, string rname, EventScheduler *e)
{

	Resource *r = dynamic_cast<Resource *>(getAuctioningObject(sname, rname));
	
	if (r != NULL)
		delResource(r, e);
}

void 
ResourceManager::delResources(string sname, EventScheduler *e)
{
	auctioningObjectIndex_t *objects = getAuctioningObjects(sname);
	auctioningObjectIndexIter_t iter;
	
    for (auctioningObjectIndexIter_t i = objects->begin(); i != objects->end(); i++) 
    {	
        Resource *r = dynamic_cast<Resource *>(getAuctioningObject(sname, i->first));
        delResource(r,e);
    }
}



void 
ResourceManager::delAuctioningObjects(auctioningObjectDB_t *resources, EventScheduler *e)
{

    auctioningObjectDBIter_t iter;

    for (iter = resources->begin(); iter != resources->end(); iter++) 
    {
        AuctioningObject * ao = *iter;
        Resource *r = dynamic_cast<Resource *>(ao); 
        delResource(r, e);
    }

}

/* ------------------------- dump ------------------------- */

void ResourceManager::dump( ostream &os )
{
    
    auctioningObjectDBIter_t iter;
    os << "Resource Manager dump :" << endl;
    
    auctioningObjectDB_t object = getAuctioningObjects();
    for (iter = object.begin(); iter != object.end(); ++iter ){
		AuctioningObject *ao = *iter;
		Resource *r = dynamic_cast<Resource *>(ao);
		os << r->getInfo() << endl;
    }
}

/* ------------------------- operator<< ------------------------- */

ostream& auction::operator<< ( ostream &os, ResourceManager &rm )
{
    rm.dump(os);
    return os;
}

/* ------------------------- getInfo ------------------------- */

string ResourceManager::getInfo(string sname, string rname)
{

    string info;
    AuctioningObject *ao;
  
    ao = getAuctioningObject(sname, rname);
    Resource *r = dynamic_cast<Resource *>(ao);

    if ( r == NULL )
    {
        // check done tasks
        AuctioningObject *ao = getAuctioningObjectDone(sname, rname);
        r = dynamic_cast<Resource *>(ao);
        if (r != NULL){
			info = r->getInfo();
        } else {
            throw Error("no auctioning object with bid name '%s.%s'", sname.c_str(), rname.c_str());
        }
    } else {
        // auction object with given identification is in database
        info = r->getInfo();
    }
    
    return info;
}

/* ------------------------- getInfo ------------------------- */

string ResourceManager::getInfo(int uid)
{ 
	AuctioningObject *ao = getAuctioningObject(uid); 
	
	Resource *r = dynamic_cast<Resource *>(ao);
	
	if (r != NULL)
		return r->getInfo();
	else
		return string();
}

/* ------------------------- getInfo ------------------------- */

string ResourceManager::getInfo(string sname)
{
    ostringstream s;

	auctioningObjectIndex_t *objects = getAuctioningObjects(sname);
	auctioningObjectIndexIter_t iter;

    for (auctioningObjectIndexIter_t i = objects->begin(); i != objects->end(); i++) {						
        Resource *r = dynamic_cast<Resource *>(getAuctioningObject(sname, i->first));
        s << r->getInfo();
    }
    
    return s.str();
}

bool ResourceManager::verifyAuctions(auctioningObjectDB_t *auctions)
{

//#ifdef DEBUG    
    log->log(ch, "starting verifyAuctions");
//#endif
	
	bool valReturn = true;
	
	// Loop through the auctions, bring thre resource and verify that 
	// inserting the auction are valid in terms of the resource intervals.  
	// Insert the resource temporarily, when the verification is done
	// it removes inserted auctions.
	auctioningObjectDBIter_t iter;
	for (iter = auctions->begin(); ((iter != auctions->end()) && (valReturn == true)); ++iter)
	{
		Auction *auction = dynamic_cast<Auction *>(*iter);
		if (auction != NULL){
			// Bring the resource
			string sResource = auction->getSetParent();
			string sName = auction->getNameParent();
			
			log->log(ch, "resource id:%s.%s", sResource.c_str(), sName.c_str() );
						
			Resource *rTmp = getResource(sResource, sName);
			if (rTmp == NULL){
				log->elog(ch, "Resource %s.%s not found in database", 
									sResource.c_str(), sName.c_str());
				valReturn = false;
			} else {
				valReturn = rTmp->verifyAuction(auction);
				
				log->log(ch, "After verifying %s", sResource.c_str(), sName.c_str(), valReturn ? "true" : "false" );
				
				if (valReturn == true){
					rTmp->addAuction(auction);
				} else { 
					log->elog(ch, "Verification error trying to include \
										auction %s.%s in resource %s.%s", 
									auction->getSet().c_str(),  
									auction->getName().c_str(),	sResource.c_str(), sName.c_str());
				}
			}
		}
	}
	
	// Delete all the auctions inserted
	for (iter = auctions->begin(); iter != auctions->end(); ++iter)
	{
		Auction *auction = dynamic_cast<Auction *>(*iter);
		if (auction != NULL){
			// Brings the resource
			string sResource = auction->getSetParent();
			string sName = auction->getNameParent();
			
			Resource *rTmp = getResource(sResource, sName);
			if (rTmp != NULL){
				rTmp->deleteAuction(auction);
			}
		}
	}

//#ifdef DEBUG    
    log->log(ch, "ending verifyAuctions %d", valReturn);
//#endif	
	return valReturn;
}
