
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

using namespace auction;

/* ------------------------- ResourceRequestManager ------------------------- */

ResourceRequestManager::ResourceRequestManager( int domain, string fdname, string fvname) 
    : FieldDefManager(fdname, fvname), resourceRequests(0), domain(domain), idSource(1)
{
    log = Logger::getInstance();
    ch = log->createChannel("ResourceRequestManager");
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

}


/* ------------------------- ~ResourceRequestManager ------------------------- */

ResourceRequestManager::~ResourceRequestManager()
{
    resourceRequestDBIter_t iter;

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

    for (iter = resourceRequestDB.begin(); iter != resourceRequestDB.end(); iter++) {
        if (*iter != NULL) {
            // delete resource request
            saveDelete(*iter);
        } 
    }

    for (resourceRequestDoneIter_t i = resourceRequestDone.begin(); 
				i != resourceRequestDone.end(); i++) {
        saveDelete(*i);
    }
}


/* --------------------- getResourceRequest ------------------------- */

ResourceRequest *ResourceRequestManager::getResourceRequest(int uid)
{
    if ((uid >= 0) && ((unsigned int)uid <= resourceRequestDB.size())) {
        return resourceRequestDB[uid];
    } else {
        return NULL;
    }
}



/* -------------------- getResourceRequest -------------------- */

ResourceRequest *
ResourceRequestManager::getResourceRequest(string sname, string rname)
{
    resourceRequestSetIndexIter_t iter;
    resourceRequestIndexIter_t iter2;

    iter = resourceRequestSetIndex.find(sname);
    if (iter != resourceRequestSetIndex.end()) {		
        iter2 = iter->second.find(rname);
        if (iter2 != iter->second.end()) {
            return getResourceRequest(iter2->second);
        }
        else
        {
#ifdef DEBUG
    log->dlog(ch,"Resource Request Id not found %s.%s",sname.c_str(), rname.c_str());
#endif		
			
		}
    }
    else
    {
#ifdef DEBUG
    log->dlog(ch,"Resource Request set not found");
#endif		
	}

    return NULL;
}


/* -------------------- getBids -------------------- */

resourceRequestIndex_t *
ResourceRequestManager::getResourceRequests(string sname)
{
    resourceRequestSetIndexIter_t iter;

    iter = resourceRequestSetIndex.find(sname);
    if (iter != resourceRequestSetIndex.end()) {
        return &(iter->second);
    }

    return NULL;
}

resourceRequestDB_t ResourceRequestManager::getResourceRequests()
{
    resourceRequestDB_t ret;

    for (resourceRequestSetIndexIter_t r = resourceRequestSetIndex.begin(); 
			r != resourceRequestSetIndex.end(); r++) {
        for (resourceRequestIndexIter_t i = r->second.begin(); 
				i != r->second.end(); i++) {
            ret.push_back(getResourceRequest(i->second));
        }
    }

    return ret;
}

/* --------------------- parseResourceRequests ----------------------- */

resourceRequestDB_t *ResourceRequestManager::parseResourceRequests(string fname)
{

#ifdef DEBUG
    log->dlog(ch,"ParseResourceRequests");
#endif

    resourceRequestDB_t *new_requests = new resourceRequestDB_t();

    try {	
		
        ResourceRequestFileParser rfp = ResourceRequestFileParser(fname);
        rfp.parse(FieldDefManager::getFieldDefs(), new_requests, &idSource);

#ifdef DEBUG
    log->dlog(ch, "resource requests parsed");
#endif

        return new_requests;

    } catch (Error &e) {

        for(resourceRequestDBIter_t i=new_requests->begin(); i != new_requests->end(); i++) {
           saveDelete(*i);
        }
        saveDelete(new_requests);
        throw e;
    }
}


/* -------------------- parseResourceRequestsBuffer -------------------- */

resourceRequestDB_t *ResourceRequestManager::parseResourceRequestsBuffer(char *buf, int len, int mapi)
{
    resourceRequestDB_t *new_requests = new resourceRequestDB_t();

    try {
				
        if (mapi) {
             TextResourceRequestParser rfp = TextResourceRequestParser(buf, len);
             rfp.parse(FieldDefManager::getFieldDefs(), new_requests, &idSource);
        } else {
            ResourceRequestFileParser rfp = ResourceRequestFileParser(buf, len);
            rfp.parse(FieldDefManager::getFieldDefs(),  new_requests, &idSource);
        }

        return new_requests;
	
    } catch (Error &e) {

        for(resourceRequestDBIter_t i=new_requests->begin(); i != new_requests->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_requests);
        throw e;
    }
}


/* ------------------------ addResourceRequests -------------------------- */

void ResourceRequestManager::addResourceRequests(resourceRequestDB_t * requests, EventScheduler *e)
{
    resourceRequestDBIter_t        iter;
    resourceRequestTimeIndex_t     start;
    resourceRequestTimeIndex_t     stop;
    resourceRequestTimeIndexIter_t iter2;
    time_t              now = time(NULL);
 

#ifdef DEBUG
    log->dlog(ch, "starting add resource requests");
#endif
 
        
    // add bids
    for (iter = requests->begin(); iter != requests->end(); iter++) {
        ResourceRequest *r = (*iter);
        
        try {

            addResourceRequest(r);
			
			// Loop though intervals to include the resource request 
			// in the start and stop iterators
			resourceReqIntervalList_t * intervals = r->getIntervals();
			resourceReqIntervalListIter_t intervals_iter;
			
			for (intervals_iter = intervals->begin(); 
					intervals_iter != intervals->end(); ++intervals_iter){
				
				resourceReq_interval_t interv_tmp = *intervals_iter;
				
				cout << "Resource Request Interval Start:" << Timeval::toString(interv_tmp.start) << endl;
				
				start[interv_tmp.start].push_back(r);
			
				if (interv_tmp.stop){
					cout << "Resource Request Interval Stop:" << Timeval::toString(interv_tmp.stop) << endl;
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
    for (iter2 = start.begin(); iter2 != start.end(); iter2++) 
    {
		cout << "aqui estoy 1" << Timeval::toString(iter2->first) << endl; 
		e->addEvent(new ActivateResourceRequestIntervalEvent(iter2->first-now, iter2->second, iter2->first));
    }
    
    // group resource request with same stop time
    for (iter2 = stop.begin(); iter2 != stop.end(); iter2++) {
		cout << "aqui estoy 2" << endl;
		e->addEvent(new RemoveResourceRequestIntervalEvent(iter2->first-now, iter2->second, iter2->first));
    }

#ifdef DEBUG    
    log->dlog(ch, "Finished adding resource requests");
#endif      

}


/* -------------------- addBid -------------------- */

void ResourceRequestManager::addResourceRequest(ResourceRequest *request)
{
  
#ifdef DEBUG    
    log->dlog(ch, "adding new resource request with name = '%s'",
              request->getResourceRequestName().c_str());
#endif  
				  
			  
    // test for presence of Set/Name combination
    // in RequestDatabase in particular set
    if (getResourceRequest(request->getResourceRequestSet(), request->getResourceRequestName())) {
        log->elog(ch, "Resource Request %s.%s already installed",
                  request->getResourceRequestSet().c_str(), request->getResourceRequestName().c_str());
        throw Error(408, "Resource Request with this name is already installed");
    }

    try {

		// Assigns the new Id.
		request->setUId(idSource.newId());

        // could do some more checks here
        request->setState(AO_VALID);

#ifdef DEBUG    
		log->dlog(ch, "Resource Request Id = '%d'", request->getUId());
#endif 

        // resize vector if necessary
        if ((unsigned int)request->getUId() >= resourceRequestDB.size()) {
            resourceRequestDB.reserve(request->getUId() * 2 + 1);
            resourceRequestDB.resize(request->getUId() + 1);
        }

        // insert Resource Request
        resourceRequestDB[request->getUId()] = request; 	

        // add new entry in index
        resourceRequestSetIndex[request->getResourceRequestSet()][request->getResourceRequestName()] = request->getUId();
	
        resourceRequests++;

#ifdef DEBUG    
    log->dlog(ch, "finish adding new Resource Request with name = '%s'",
              request->getResourceRequestName().c_str());
#endif  

    } catch (Error &e) { 

        // adding new Resource request failed in some component
        // something failed -> remove Resource Request from database
        delResourceRequest(request->getResourceRequestSet(), 
							request->getResourceRequestName(), NULL);
	
        throw e;
    }
}

void ResourceRequestManager::activateResourceRequests(resourceRequestDB_t *requests, EventScheduler *e)
{
    resourceRequestDBIter_t             iter;

    for (iter = requests->begin(); iter != requests->end(); iter++) {
        ResourceRequest *r = (*iter);
        log->dlog(ch, "activate resource request with name = '%s'", r->getResourceRequestName().c_str());
        r->setState(AO_ACTIVE);
		// TODO AM: Finish this code.
    }
}


/* ------------------------- getInfo ------------------------- */

string ResourceRequestManager::getInfo(ResourceRequest *r)
{
    ostringstream s;

#ifdef DEBUG
    log->dlog(ch, "looking up Bid with uid = %d", r->getUId());
#endif

    s << r->getInfo() << endl;
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string ResourceRequestManager::getInfo(string sname, string rname)
{
    ostringstream s;
    string info;
    ResourceRequest *r;
  
    r = getResourceRequest(sname, rname);

    if (r == NULL) {
        // check done tasks
        for (resourceRequestDoneIter_t i = resourceRequestDone.begin(); i != resourceRequestDone.end(); i++) {
            if (((*i)->getResourceRequestName() == rname) 
						&& ((*i)->getResourceRequestSet() == sname)) {
                info = (*i)->getInfo();
            }
        }
        
        if (info.empty()) {
            throw Error("no Resource Request with name '%s.%s'", sname.c_str(), rname.c_str());
        }
    } else {
        // Resource Request with given identification is in database
        info = r->getInfo();
    }
    
    s << info;

    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string ResourceRequestManager::getInfo(string sname)
{
    ostringstream s;
    resourceRequestSetIndexIter_t b;

    b = resourceRequestSetIndex.find(sname);

    if (b != resourceRequestSetIndex.end()) {
        for (resourceRequestIndexIter_t i = b->second.begin(); i != b->second.end(); i++) {
            s << getInfo(sname, i->first);
        }
    } else {
        s << "No such set" << endl;
    }
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string ResourceRequestManager::getInfo()
{
    ostringstream s;
    resourceRequestSetIndexIter_t iter;

    for (iter = resourceRequestSetIndex.begin(); iter != resourceRequestSetIndex.end(); iter++) {
        s << getInfo(iter->first);
    }
    
    return s.str();
}


/* ------------------------- delBid ------------------------- */

void ResourceRequestManager::delResourceRequest(string sname, string rname, EventScheduler *e)
{
    ResourceRequest *r;

#ifdef DEBUG    
    log->dlog(ch, "Deleting Resource Request set= %s name = '%s'",
              sname.c_str(), rname.c_str());
#endif  


    if (sname.empty() && rname.empty()) {
        throw Error("incomplete Resource Request set or name specified");
    }

    r = getResourceRequest(sname, rname);

    if (r != NULL) {
        delResourceRequest(r, e);
    } else {
        throw Error("Resouce Request %s.%s does not exist", sname.c_str(),rname.c_str());
    }
}


/* ---------------------- delResourceRequest ----------------------- */

void ResourceRequestManager::delResourceRequest(int uid, EventScheduler *e)
{
    ResourceRequest *r;

    r = getResourceRequest(uid);

    if (r != NULL) {
        delResourceRequest(r, e);
    } else {
        throw Error("Resource Request uid %d does not exist", uid);
    }
}


/* ---------------------- delResourceRequests ----------------------- */

void ResourceRequestManager::delResourceRequests(string sname, EventScheduler *e)
{
    
    if (resourceRequestSetIndex.find(sname) != resourceRequestSetIndex.end()) 
    {
		resourceRequestSetIndexIter_t iter = resourceRequestSetIndex.find(sname);
		resourceRequestIndex_t resourceRequestIndex = iter->second;
        for (resourceRequestIndexIter_t i = resourceRequestIndex.begin(); 
				i != resourceRequestIndex.end(); i++) 
        {
            delResourceRequest(getResourceRequest(sname, i->first),e);
        }
    }
}


/* ------------------------- delBid ------------------------- */

void ResourceRequestManager::delResourceRequest(ResourceRequest *r, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "removing resource request with name = '%s'", r->getResourceRequestName().c_str());
#endif

	if( EventSchedulerAgent * eagent = dynamic_cast<EventSchedulerAgent*>(e) )
	{
		// remove bid from database and from index
		storeResourceRequestAsDone(r);
		resourceRequestDB[r->getUId()] = NULL;
		resourceRequestSetIndex[r->getResourceRequestSet()].erase(r->getResourceRequestName());

		// delete bid set if empty
		if (resourceRequestSetIndex[r->getResourceRequestSet()].empty()) {
			resourceRequestSetIndex.erase(r->getResourceRequestName());
		}
		
		if (eagent != NULL) {
			eagent->delResourceRequestEvents(r->getUId());
		}

		resourceRequests--;
	}
	else {
		throw Error("Event scheduler given is not of type agent");
	}
}


/* ----------------------- delResourceRequests ----------------------- */

void ResourceRequestManager::delResourceRequests(resourceRequestDB_t *requests, EventScheduler *e)
{
    resourceRequestDBIter_t iter;

    for (iter = requests->begin(); iter != requests->end(); iter++) {
        delResourceRequest(*iter, e);
    }
}


/* -------------------- storeResourceRequestAsDone -------------------- */

void ResourceRequestManager::storeResourceRequestAsDone(ResourceRequest *r)
{
    
    r->setState(AO_DONE);
    resourceRequestDone.push_back(r);

    if (resourceRequestDone.size() > DONE_LIST_SIZE) {
        // release id
        idSource.freeId(resourceRequestDone.front()->getUId());
        // remove Resource Request
        saveDelete(resourceRequestDone.front());
        resourceRequestDone.pop_front();
    }
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
