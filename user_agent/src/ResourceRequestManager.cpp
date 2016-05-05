
/*! \file   ResourceRequestManager.cpp

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

    $Id: ResourceRequestManager.h 748 2015-08-25 16:25:00Z amarentes $

*/

#include "ParserFcts.h"
#include "ResourceRequestManager.h"
#include "Constants.h"
#include "EventAgent.h"
#include "TextResourceRequestParser.h"
#include "MAPIResourceRequestParser.h"
#include "AuctioningObjectManager.h"

using namespace auction;

/* ------------------------- ResourceRequestManager ------------------------- */

ResourceRequestManager::ResourceRequestManager( int domain, string fdname, string fvname) 
    : AuctioningObjectManager(domain, fdname, fvname, "ResourceRequestManager")     
{

#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

}


/* ------------------------- ~ResourceRequestManager ------------------------- */

ResourceRequestManager::~ResourceRequestManager()
{

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

}


/* --------------------- getResourceRequest ------------------------- */

ResourceRequest *ResourceRequestManager::getResourceRequest(int uid)
{
    AuctioningObject *ao = getAuctioningObject(uid);
    
    if (ao != NULL)
		return dynamic_cast<ResourceRequest *>(ao);
	else
		return NULL;
		
}



/* -------------------- getResourceRequest -------------------- */

ResourceRequest *
ResourceRequestManager::getResourceRequest(string sname, string rname)
{

    AuctioningObject *ao = getAuctioningObject(sname, rname);
    
    if (ao != NULL)
		return dynamic_cast<ResourceRequest *>(ao);
	else
		return NULL;

}


/* --------------------- parseResourceRequests ----------------------- */

auctioningObjectDB_t *
ResourceRequestManager::parseResourceRequests(string fname)
{

#ifdef DEBUG
    log->dlog(ch,"ParseResourceRequests");
#endif

    auctioningObjectDB_t *new_requests = new auctioningObjectDB_t();

    try {	
		
        ResourceRequestFileParser rfp = ResourceRequestFileParser(fname);
        rfp.parse(FieldDefManager::getFieldDefs(), new_requests);

#ifdef DEBUG
    log->dlog(ch, "resource requests parsed");
#endif

        return new_requests;

    } catch (Error &e) {

        for(auctioningObjectDBIter_t i=new_requests->begin(); i != new_requests->end(); i++) {
           saveDelete(*i);
        }
        saveDelete(new_requests);
        throw e;
    }
}


/* -------------------- parseResourceRequestsBuffer -------------------- */

auctioningObjectDB_t *
ResourceRequestManager::parseResourceRequestsBuffer(char *buf, int len, int mapi)
{
    auctioningObjectDB_t *new_requests = new auctioningObjectDB_t();

    try {
				
        if (mapi) {
             TextResourceRequestParser rfp = TextResourceRequestParser(buf, len);
             rfp.parse(FieldDefManager::getFieldDefs(), new_requests);
        } else {
            ResourceRequestFileParser rfp = ResourceRequestFileParser(buf, len);
            rfp.parse(FieldDefManager::getFieldDefs(),  new_requests);
        }

        return new_requests;
	
    } catch (Error &e) {

        for(auctioningObjectDBIter_t i=new_requests->begin(); i != new_requests->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_requests);
        throw e;
    }
}


/* ------------------------ addResourceRequests -------------------------- */

void ResourceRequestManager::addAuctioningObjects(auctioningObjectDB_t * requests, EventScheduler *e)
{
    auctioningObjectDBIter_t       iter;
    resourceRequestTimeIndex_t     start;
    resourceRequestTimeIndex_t     stop;
    resourceRequestTimeIndexIter_t iter2;
    time_t              now = time(NULL);
 

#ifdef DEBUG
    log->dlog(ch, "starting add resource requests");
#endif
 
        
    // add bids
    for (iter = requests->begin(); iter != requests->end(); iter++) 
    {    
        ResourceRequest *r = dynamic_cast<ResourceRequest *>(*iter);
        try {

            addAuctioningObject(r);
			
			// Loop though intervals to include the resource request 
			// in the start and stop iterators
			resourceReqIntervalList_t * intervals = r->getIntervals();
			resourceReqIntervalListIter_t intervals_iter;
			
			for (intervals_iter = intervals->begin(); 
					intervals_iter != intervals->end(); ++intervals_iter){
				
				resourceReq_interval_t interv_tmp = *intervals_iter;								
				start[interv_tmp.start].push_back(r);
			
				if (interv_tmp.stop){
					stop[interv_tmp.stop].push_back(r);
				}
			}
        } catch (Error &e ) {
            saveDelete(r);
            // if only one rule return error
            if (requests->size() == 1) {
                throw e;
            }
            // FIXME else return number of successively installed Resource Request
        }
    }
    
#ifdef DEBUG    
    log->dlog(ch, "Start all resoruce request - it is going to activate them");
#endif      

    // group resource requests with same start time
    time_t usec = 1;
    for (iter2 = start.begin(); iter2 != start.end(); iter2++) 
    {
		auctioningObjectDBIter_t reqDBIter;
		for (reqDBIter = (iter2->second).begin(); reqDBIter != (iter2->second).end(); ++reqDBIter ){
			ResourceRequest *req = dynamic_cast<ResourceRequest *>(*reqDBIter);
			e->addEvent(new ActivateResourceRequestIntervalEvent(iter2->first-now, usec, req, iter2->first));
			usec = usec + 1;
		}
    }
    
    // group resource request with same stop time
    usec = 1;
    for (iter2 = stop.begin(); iter2 != stop.end(); iter2++) {
		auctioningObjectDBIter_t reqDBIter;
		for (reqDBIter = (iter2->second).begin(); reqDBIter != (iter2->second).end(); ++reqDBIter )
		{
			ResourceRequest *req = dynamic_cast<ResourceRequest *>(*reqDBIter);
			e->addEvent(new RemoveResourceRequestIntervalEvent(iter2->first-now, usec, req, iter2->first));
			usec = usec + 1;
		}
    }

#ifdef DEBUG    
    log->dlog(ch, "Finished adding resource requests");
#endif      

}


/* ------------------------- delBid ------------------------- */

void ResourceRequestManager::delResourceRequest(ResourceRequest *r, EventScheduler *e)
{
	
#ifdef DEBUG    
    log->dlog(ch, "removing resource request with name = %s.%s", 
					r->getSet().c_str(), r->getName().c_str());
#endif

	if( EventSchedulerAgent * eagent = dynamic_cast<EventSchedulerAgent*>(e) )
	{
		
		AuctioningObjectManager::delAuctioningObject(r);

		if (eagent != NULL) {
			eagent->delResourceRequestEvents(r->getUId());
		}

	}
	else {
		throw Error("Event scheduler given is not of type agent");
	}
}


/* ---------------------- delResourceRequest ----------------------- */

void ResourceRequestManager::delResourceRequest(int uid, EventScheduler *e)
{
    ResourceRequest *r;

    r = dynamic_cast<ResourceRequest *>(getAuctioningObject(uid)); 

    if (r != NULL) {
        delResourceRequest(r, e);
    } else {
        throw Error("Resource Request uid %d does not exist", uid);
    }
}


/* ----------------------- delResourceRequest ---------------------- */

void ResourceRequestManager::delResourceRequest(string sname, string rname, EventScheduler *e)
{
    ResourceRequest *r;

#ifdef DEBUG    
    log->dlog(ch, "Deleting Resource Request %s.%s", sname.c_str(), rname.c_str());
#endif  


    if (sname.empty() && rname.empty()) {
        throw Error("incomplete Resource Request set or name specified");
    }

    r =  dynamic_cast<ResourceRequest *>(getAuctioningObject(sname, rname)); 

    if (r != NULL) {
        delResourceRequest(r, e);
    } else {
        throw Error("Resouce Request %s.%s does not exist", sname.c_str(),rname.c_str());
    }
}



/* ---------------------- delResourceRequests ----------------------- */

void ResourceRequestManager::delResourceRequests(string sname, EventScheduler *e)
{
    
	auctioningObjectIndex_t *objects = getAuctioningObjects(sname);
	auctioningObjectIndexIter_t iter;
	
    for (auctioningObjectIndexIter_t i = objects->begin(); i != objects->end(); i++) 
    {						
        ResourceRequest *r = dynamic_cast<ResourceRequest *>(getAuctioningObject(sname, i->first));
        delResourceRequest(r,e);
    }
    
}



/* ----------------------- delResourceRequests ----------------------- */

void ResourceRequestManager::delAuctioningObjects(auctioningObjectDB_t *requests, EventScheduler *e)
{
    
    auctioningObjectDBIter_t iter;

    for (iter = requests->begin(); iter != requests->end(); iter++) {
        AuctioningObject * ao = *iter;
        ResourceRequest *r = dynamic_cast<ResourceRequest *>(ao); 
        delResourceRequest(r, e);
    }
}

/* ------------------------- getInfo ------------------------- */

string AuctionManager::getInfo(int uid)
{ 
	AuctioningObject *ao = getAuctioningObject(uid); 
	
	ResourceRequest *r = dynamic_cast<ResourceRequest *>(ao);
	
	if (r != NULL)
		return r->getInfo();
	else
		return string();
}


/* ------------------------- getInfo ------------------------- */

string ResourceRequestManager::getInfo(string sname, string rname)
{

    string info;
    AuctioningObject *ao;
  
    ao = getAuctioningObject(sname, rname);
    ResourceRequest *r = dynamic_cast<ResourceRequest *>(ao);

    if (r == NULL) 
    {
        // check done tasks
        AuctioningObject *ao = getAuctioningObjectDone(sname, rname);
        r = dynamic_cast<ResourceRequest *>(ao);
        if (r != NULL){
			info = r->getInfo();
        } else {
            throw Error("no auctioning object with name '%s.%s'", sname.c_str(), rname.c_str());
        }
    } else {
        // auction object with given identification is in database
        info = r->getInfo();
    }
    
    return info;



}


/* ------------------------- getInfo ------------------------- */

string ResourceRequestManager::getInfo(string sname)
{

    ostringstream s;

	auctioningObjectIndex_t *objects = getAuctioningObjects(sname);
	auctioningObjectIndexIter_t iter;

    for (auctioningObjectIndexIter_t i = objects->begin(); i != objects->end(); i++) {						
        ResourceRequest *r = dynamic_cast<ResourceRequest *>(getAuctioningObject(sname, i->first));
        s << r->getInfo();
    }
    
    return s.str();


}


/* ------------------------- getInfo ------------------------- */

string ResourceRequestManager::getInfo()
{
    ostringstream s;
    auctioningObjectDBIter_t iter;

    for (iter = getAuctioningObjects().begin(); iter != getAuctioningObjects().end(); iter++) 
    {
        ResourceRequest *request = dynamic_cast<ResourceRequest *>(*iter);
        s << request->getInfo();
    }
    
    return s.str();
}


ipap_message * 
ResourceRequestManager::get_ipap_message(ResourceRequest *request, time_t start,
										 string resourceId, bool useIPV6, 
										 string sAddressIPV4, string sAddressIPV6, uint16_t port)
{

	MAPIResourceRequestParser mrrp = MAPIResourceRequestParser(getDomain());

	return mrrp.get_ipap_message(FieldDefManager::getFieldDefs(), 
								 request, start, resourceId, 
								 useIPV6, sAddressIPV4, sAddressIPV6, port);


}


/* ------------------------- dump ------------------------- */

void ResourceRequestManager::dump( ostream &os )
{
    
    os << "ResourceRequestManager dump :" << endl;
    os << getInfo() << endl;
    
}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, ResourceRequestManager &rm )
{
    rm.dump(os);
    return os;
}
