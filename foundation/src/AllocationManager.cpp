
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

#include "ParserFcts.h"
#include "AllocationManager.h"
#include "MAPIAllocationParser.h"
#include "Constants.h"

using namespace auction;

/* ------------------------- AllocationManager ------------------------- */

AllocationManager::AllocationManager( string fdname, string fvname) 
    : FieldDefManager(fdname, fvname), allocations(0), idSource(1)
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


/* -------------------------- getAllocation ----------------------------- */

Allocation *AllocationManager::getAllocation(int uid)
{
    if ((uid >= 0) && ((unsigned int)uid <= allocationDB.size())) {
        return allocationDB[uid];
    } else {
        return NULL;
    }
}



/* -------------------- getAllocation -------------------- */

Allocation *AllocationManager::getAllocation(string aset, string aname)
{
    allocationSetIndexIter_t iter;
    allocationIndexIter_t iter2;

    iter = allocationSetIndex.find(aset);
    if (iter != allocationSetIndex.end()) 
    {        
        
        iter2 = iter->second.find(aname);
        if (iter2 != iter->second.end())
        {
			allocationUIDIndexIter_t list_inter;
			for (list_inter = iter2->second.begin(); list_inter != iter2->second.end(); ++list_inter)
			{
				Allocation * allocation = getAllocation(list_inter->first);
				if ( ( aset.compare(allocation->getAllocationSet() ) == 0 ) &&
				    ( aname.compare(allocation->getAllocationName() ) == 0 ) )
				{
				    return  allocation;
				}  
			}
#ifdef DEBUG
			log->dlog(ch,"Allocation not found %s.%s", aset.c_str(), aname.c_str());
#endif			
		}
		else
		{
		
#ifdef DEBUG
    log->dlog(ch,"Allocation Name not found %s.%s", aset.c_str(), aname.c_str());
#endif		
			
		}
    }
    else
    {
#ifdef DEBUG
    log->dlog(ch,"AllocationSet not found %s", aset.c_str());
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


/* -------------------- parseMessage -------------------- */

allocationDB_t *
AllocationManager::parseMessage(ipap_message *messageIn, ipap_template_container *templates)
{
    allocationDB_t *new_allocations = new allocationDB_t();

    try {
			
        MAPIAllocationParser afp = MAPIAllocationParser(getDomain());
        
        afp.parse(FieldDefManager::getFieldDefs(), 
					FieldDefManager::getFieldVals(), 
						messageIn, new_allocations, templates);

        return new_allocations;
	
    } catch (Error &e) {

        for(allocationDBIter_t i=new_allocations->begin(); i != new_allocations->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_allocations);
        throw e;
    }
}



/* ---------------------------- addAllocations ----------------------------- */

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
            if (_allocations->size() == 1) {
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
 					  (alloc->getAuctionSet()).c_str(), 
				      (alloc->getAuctionName()).c_str());
#endif 					
			// TODO AM: Create an Activation Allocation Event.
			//e->addEvent(new InsertAllocationEvent(iter2->first-now, alloc);
		}
    }
    
    // group allocations with same stop time
    for (iter2 = stop.begin(); iter2 != stop.end(); iter2++) {
		allocationDBIter_t allocs_iter;
		// Iterates over the allocations stoping.
		for (allocs_iter = (iter2->second).begin(); 
				allocs_iter != (iter2->second).end(); allocs_iter++) {
			Allocation *alloc = (*allocs_iter); 

			// TODO AM: Create a remove Allocation Event.
			//e->addEvent(new RemoveAllocationEvent(iter2->first-now, alloc );
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
    log->dlog(ch, "adding new allocation with name = %s.%s",
              a->getAllocationSet().c_str(), 
              a->getAllocationName().c_str());
#endif  
				  
			  
    // test for presence of allocationSource/allocationName combination
    // in allocation database in particular set
    if (getAllocation(a->getAllocationSet(), a->getAllocationName())) {
        log->elog(ch, "Allocation %s.%s already installed",
                  a->getAllocationSet().c_str(), a->getAllocationName().c_str() );
        throw Error(408, "Allocation with this name is already installed");
    }

    try {

		// Assigns the new Id.
		a->setUId(idSource.newId());

        // could do some more checks here
        a->setState(AO_VALID);

#ifdef DEBUG    
		log->dlog(ch, "Allocation Id = %d", a->getUId());
#endif 

        // resize vector if necessary
        if ((unsigned int)a->getUId() >= allocationDB.size()) {
            allocationDB.reserve(a->getUId() * 2 + 1);
            allocationDB.resize(a->getUId() + 1);
        }

        // insert allocation
        allocationDB[a->getUId()] = a; 	

        // add new entry in allocation index
        allocationSetIndex[a->getAllocationSet()][a->getAllocationName()][a->getUId()] = a->getUId() ;

        // add new entry in auction index
        auctionSetIndex[a->getAuctionSet()][a->getAuctionName()][a->getUId()] = a->getUId() ;
	
        // add new entry in bid index
        bidSetIndex[a->getBidSet()][a->getBidName()][a->getUId()] = a->getUId();
		
        allocations++;


#ifdef DEBUG 
    // Loop through allocations to print them.
    allocationSetIndexIter_t iter34;
    for (iter34 = allocationSetIndex.begin(); iter34 != allocationSetIndex.end(); ++iter34 ){
		log->dlog(ch, "Allocation set:%s", iter34->first.c_str());
        // Loop through allocations to print them.
        allocationIndexIter_t iter24;
        for (iter24 = (iter34->second).begin(); iter24 != (iter34->second).end(); ++iter24 ){
			log->dlog(ch, "Allocation name:%s", iter24->first.c_str());
			allocationUIDIndexIter_t iter54;
			for (iter54 = (iter24->second).begin(); iter54 != (iter24->second).end(); ++iter54){
				log->dlog(ch, "Allocation Id:%d", iter54->first);
			}
		}
	}
#endif


#ifdef DEBUG    
    log->dlog(ch, "finish adding new allocation with name = %s.%s",
              a->getAllocationSet().c_str(), a->getAllocationName().c_str() );
#endif  

    } catch (Error &e) { 

        // adding new allocation failed in some component
        // something failed -> remove allocation from database
        delAllocation(a->getAllocationSet(), a->getAllocationName(), NULL);
	
        throw e;
    }
}

void AllocationManager::activateAllocations(allocationDB_t *allocations, EventScheduler *e)
{

#ifdef DEBUG    
    log->dlog(ch, "Starting activateAllocations");
#endif  
    
    allocationDBIter_t             iter;
    for (iter = allocations->begin(); iter != allocations->end(); iter++) {
        Allocation *a = (*iter);
        log->dlog(ch, "activate allocation with name = %s.%s", a->getAllocationSet().c_str(), 
					a->getAllocationName().c_str());
        a->setState(AO_ACTIVE);
	 
        /* TODO AM: Create code to activate sessions */
    }

#ifdef DEBUG    
    log->dlog(ch, "Ending activateAllocations");
#endif  


}


/* ------------------------- getInfo ------------------------- */

string AllocationManager::getInfo(Allocation *a)
{
    ostringstream s;

#ifdef DEBUG
    log->dlog(ch, "looking up Allocation with uid = %d", a->getUId());
#endif
	Allocation *result = getAllocation(a->getUId());
    s << result->getInfo();
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string AllocationManager::getInfo(string aset, string aname)
{
    ostringstream s;
    string info;
    Allocation *a;
  
    a = getAllocation(aset, aname);

    if (a == NULL) {
        // check done tasks
        for (allocationDoneIter_t i = allocationDone.begin(); i != allocationDone.end(); i++) {
            if (((*i)->getAllocationSet() == aset) && ((*i)->getAllocationName() == aname) ) {
                info = (*i)->getInfo();
            }
        }
        
        if (info.empty()) {
            throw Error("no allocation with name %s.%s", aset.c_str(), aname.c_str());
        }
    } else {
        // allocation with given identification is in database
        info = a->getInfo();
    }
    
    s << info;

    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string AllocationManager::getInfo()
{
    ostringstream s;
    allocationDBIter_t             iter;
    
    for (iter = allocationDB.begin(); iter != allocationDB.end(); iter++) {
        s << getInfo(*iter);
    }
    
    return s.str();
}

/* ------------------------- delAllocation ------------------------- */

void AllocationManager::delAllocation(string aset, string aname, EventScheduler *e)
{
    Allocation *a;

#ifdef DEBUG    
    log->dlog(ch, "Deleting allocation allocationset= %s allocationname = %s",
              aset.c_str(), aname.c_str());
#endif  


    if (aset.empty() && aname.empty()) {
        throw Error("incomplete allocation auction set or name specified");
    }
    
    a = getAllocation(aset, aname);

    if (a != NULL) {
        delAllocation(a, e);
    } else {
        throw Error("Allocation %s.%s does not exist", aset.c_str(), 
						aname.c_str() );
    }
}


/* ------------------------- delAllocation ------------------------- */

void AllocationManager::delAllocation(int uid, EventScheduler *e)
{
    Allocation *a;

    a = getAllocation(uid);

    if (a != NULL) {
        delAllocation(a, e);
    } else {
        throw Error("Allocation uid %d does not exist", uid);
    }
}



/* ------------------------- delBidAllocations ------------------------- */

void AllocationManager::delBidAllocations(string bset, string bname, EventScheduler *e)
{

#ifdef DEBUG
    log->dlog(ch,"Starting delBidAllocations %s.%s", bset.c_str(), bname.c_str());
#endif
    
    
    if (bidSetIndex.find(bset) != bidSetIndex.end()) 
    {
		allocationSetIndexIter_t iter = bidSetIndex.find(bset);
		allocationIndex_t allocSetIndex = iter->second;
		
		if (allocSetIndex.find(bname) != allocSetIndex.end()){
			allocationIndexIter_t allocIndex = allocSetIndex.find(bname);
			
			allocationUIDIndex_t uidsVector = allocIndex->second;
			allocationUIDIndexIter_t uidVectorIter; 
			for (uidVectorIter = uidsVector.begin(); uidVectorIter != uidsVector.end(); uidVectorIter++) 
			{				 
				Allocation *alloc = getAllocation(uidVectorIter->first);
				delAllocation(alloc,e);
			}
		}
    }


#ifdef DEBUG
    log->dlog(ch,"Ending delBidAllocations %s.%s", bset.c_str(), bname.c_str());
#endif
    
}

/* ------------------------- delBidAllocations ------------------------- */

void AllocationManager::delAuctionAllocations(string aset, string aname, EventScheduler *e)
{

#ifdef DEBUG
    log->dlog(ch,"Starting delAuctionAllocations %s.%s", aset.c_str(), aname.c_str());
#endif
    
    if (auctionSetIndex.find(aset) != auctionSetIndex.end()) 
    {
		allocationSetIndexIter_t iter = auctionSetIndex.find(aset);
		allocationIndex_t allocSetIndex = iter->second;
		if (allocSetIndex.find(aname) != allocSetIndex.end()){
			allocationIndexIter_t allocIndex = allocSetIndex.find(aname);
			allocationUIDIndex_t uidsVector = allocIndex->second;
			allocationUIDIndexIter_t uidVectorIter; 
			
			for (uidVectorIter = uidsVector.begin(); uidVectorIter != uidsVector.end(); uidVectorIter++) 
			{
				delAllocation(uidVectorIter->first,e);
			}
		}
    }

#ifdef DEBUG
    log->dlog(ch,"Ending delAuctionAllocations %s.%s", aset.c_str(), aname.c_str());
#endif


}


/* ------------------------- delAllocation ------------------------- */

void AllocationManager::delAllocation(Allocation *a, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "removing allocation with name = %s.%s", a->getAllocationSet().c_str(), 
					a->getAllocationName().c_str());
#endif

    // remove allocation from database
    storeAllocationAsDone(a);
    allocationDB[a->getUId()] = NULL;
        
        
    // remove allocation from auction index
    if (auctionSetIndex.find(a->getAuctionSet()) != auctionSetIndex.end()) 
    {
		allocationSetIndexIter_t iterAucSet = auctionSetIndex.find(a->getAuctionSet());
		allocationIndex_t allocSetIndexAuc = iterAucSet->second;
		
		if (allocSetIndexAuc.find(a->getAuctionName()) != allocSetIndexAuc.end())
		{
			allocationIndexIter_t allocIndexAuc = allocSetIndexAuc.find(a->getAuctionName());
			allocationUIDIndex_t uidsVectorAuc = allocIndexAuc->second;
			
			if (uidsVectorAuc.find(a->getUId()) != uidsVectorAuc.end())
			{
				(auctionSetIndex[a->getAuctionSet()][a->getAuctionName()]).erase(a->getUId());
			}
		}
		
		// Delete auction name, if the list went to empty.
		allocationIndexIter_t allocIndexAuc = allocSetIndexAuc.find(a->getAuctionName());
		if ((allocIndexAuc->second).empty()){
			(auctionSetIndex[a->getAuctionSet()]).erase(a->getAuctionName());
		}
    }
        
    // remove allocation from bid index
    if (bidSetIndex.find(a->getBidSet()) != bidSetIndex.end()) 
    {
		allocationSetIndexIter_t iter = bidSetIndex.find(a->getBidSet());
		allocationIndex_t allocSetIndex = iter->second;
		
		if (allocSetIndex.find(a->getBidName()) != allocSetIndex.end()){
			allocationIndexIter_t allocIndex = allocSetIndex.find(a->getBidName());
			
			allocationUIDIndex_t uidsVector = allocIndex->second;
			if (uidsVector.find(a->getUId()) != uidsVector.end())
			{	
				(bidSetIndex[a->getBidSet()][a->getBidName()]).erase(a->getUId());
			}			
		}
		
		// Delete bid name, if the list went to empty
		allocationIndexIter_t allocIndex = allocSetIndex.find(a->getBidName());
		if ((allocIndex->second).empty()){
			(bidSetIndex[a->getBidSet()]).erase(a->getBidName());
		}
		
    }
    
    // remove allocation from the allocation index 
    if (allocationSetIndex.find(a->getAllocationSet()) != allocationSetIndex.end()) 
    {
		allocationSetIndexIter_t iter = allocationSetIndex.find(a->getAllocationSet());
		allocationIndex_t allocSetIndex = iter->second;
		
		if (allocSetIndex.find(a->getAllocationName()) != allocSetIndex.end())
		{
			allocationIndexIter_t allocIndex = allocSetIndex.find(a->getAllocationName());	
			allocationUIDIndex_t uidsVector = allocIndex->second;
			if (uidsVector.find(a->getUId()) != uidsVector.end()){
				(allocationSetIndex[a->getAllocationSet()][a->getAllocationName()]).erase(a->getUId());
			}			
		}
		
		// Delete allocation name, if the list went to empty
		allocationIndexIter_t allocIndex = allocSetIndex.find(a->getAllocationName());
		if ((allocIndex->second).empty())
		{
			(allocationSetIndex[a->getAllocationSet()]).erase(a->getAllocationName());
		}

    }
      
    // delete bid set if empty
    if (bidSetIndex[a->getBidSet()].empty()) {
        bidSetIndex.erase(a->getBidSet());
    }
    
    // delete auction set if empty
    if (auctionSetIndex[a->getAuctionSet()].empty()) {
        auctionSetIndex.erase(a->getAuctionSet());
    }

    // delete allocation set if empty
    if (allocationSetIndex[a->getAllocationSet()].empty()) {
        auctionSetIndex.erase(a->getAllocationSet());
    }
      
    if (e != NULL) {
        // TODO AM: Create the eliminate Allocation events
        //e->delAllocationEvents(a->getUId());
    }

    allocations--;
}


/* ------------------------- delAllocations ------------------------- */

void AllocationManager::delAllocations(allocationDB_t *allocations, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "Starting delAllocations by vector" );
#endif


    allocationDBIter_t iter;

    for (iter = allocations->begin(); iter != allocations->end(); iter++) {
        delAllocation( (*iter)->getAllocationSet(), (*iter)->getAllocationName(), e);
    }

#ifdef DEBUG    
    log->dlog(ch, "Ending delAllocations by vector" );
#endif

}


/* -------------------- storeBidAsDone -------------------- */

void AllocationManager::storeAllocationAsDone(Allocation *a)
{
    
    a->setState(AO_DONE);
    allocationDone.push_back(a);

    if (allocationDone.size() > DONE_LIST_SIZE) {
        // release id
        idSource.freeId(allocationDone.front()->getUId());
        // remove allocation
        saveDelete(allocationDone.front());
        allocationDone.pop_front();
    }
}


/* ------------------------- dump ------------------------- */

void AllocationManager::dump( ostream &os )
{
    
    os << "AllocationManager dump :" << endl;
    os << getInfo() << endl;
    
}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, AllocationManager &rm )
{
    rm.dump(os);
    return os;
}
