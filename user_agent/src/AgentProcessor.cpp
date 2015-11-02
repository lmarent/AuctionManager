
/*!\file   AgentProcessor.cpp

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
		executes the auction processing functions for an agent.

    $Id: AgentProcessor.cpp 748 2015-08-25 11:25:00Z amarentes $
*/


#include "ParserFcts.h"
#include <stdinc.h>
#include "AgentProcessor.h"

using namespace auction;

/* ------------------------- AUMProcessor ------------------------- */

AgentProcessor::AgentProcessor(ConfigManager *cnf, string fdname, string fvname, int threaded, string moduleDir ) 
    : AuctionManagerComponent(cnf, "AGENT_PROCESSOR", threaded), 
	  fieldDefManager(fdname, fvname), idSource(0)
{
    string txt;
    
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

    if (moduleDir.empty()) {
		txt = cnf->getValue("ModuleDir", "AGNT_PROCESSOR");
        if ( txt != "") {
            moduleDir = txt;
        }        
    }

    try {
        loader = new ModuleLoader(cnf, moduleDir.c_str() /*module (lib) basedir*/,
                                  cnf->getValue("Modules", "AGNT_PROCESSOR"),/*modlist*/
                                  "Proc" /*channel name prefix*/,
                                  getConfigGroup() /* Configuration group */);

#ifdef DEBUG
    log->dlog(ch,"End starting");
#endif

    } catch (Error &e) {
        throw e;
    }
	
}


/* ------------------------- ~AUMProcessor ------------------------- */

AgentProcessor::~AgentProcessor()
{

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

#ifdef ENABLE_THREADS
    if (threaded) {
        mutexLock(&maccess);
        stop();
        mutexUnlock(&maccess);
        mutexDestroy(&maccess);
    }
#endif
		
}

/* ------------------------ addRequest -------------------------*/
int AgentProcessor::addRequest( fieldList_t *parameters, auctionDB_t *auctions, EventScheduler *e )
{

#ifdef DEBUG
    log->dlog(ch,"Start addRequest");
#endif

	Module *mod;
	string sModuleName;
	int index = -2;
	map<string, auctionDB_t *> splitByModule;
	map<string, auctionDB_t *>::iterator splitByModuleIter;
    bool exThrown = false;	

	// Verifies that auctions must be greater than zero and different from null
	if (auctions == NULL){
		throw Error("Auction pointer given for add request function is NULL");
	}
	
	if ( auctions->size() == 0 ){
		throw Error("Number of auction given for add request function is zero");
	}

    AUTOLOCK(threaded, &maccess);  


	try {
				
		// Go through the list of auctions and create groups by their module 
		auctionDBIter_t auctIter;
		for (auctIter = auctions->begin(); auctIter != auctions->begin(); ++auctIter)
		{	
			// Read the name of the module to load
			sModuleName = auctIter->getAction()->name;
			splitByModuleIter = splitByModule.find(sModuleName);
			if (splitByModuleIter == splitByModule.end()){
				splitByModule[sModuleName] = new auctionDB_t();
			}
			// Insert a pointer to the auction.
			(splitByModule[sModuleName])->push_back(*auctIter);			
		} 
		
		index = idSource.newId();
		
		// Go through the map of modules to load them and create the necessary request processes.
		for (splitByModuleIter = splitByModule.begin(); splitByModuleIter != splitByModule.end();  ++splitByModuleIter)
		{

			requestProcess_t reqProcess;
			
			// load the module
			mod = loader->getModule(splitByModuleIter->first); 
			reqProcess.module = dynamic_cast<ProcModule*> (mod); 
			if (reqProcess.module != NULL) { // is it a processing kind of module
				reqProcess.mapi = reqProcess.module->getAPI();

				// init module
				reqProcess.index = index;
				reqProcess.parameters = parameters;
				reqProcess.auctions = splitByModuleIter->second;
			}
			
			// Insert in the list of requests needing processing
			requests[index] = reqProcess; 	
		}
		  
    catch (Error &e) 
    { 
        log->elog(ch, e);
        errNo = e.getErrorNo();
        errStr = e.getError();
		exThrown = true;	
    }
	
	catch (ProcError &e)
	{
        log->elog(ch, e.getError().c_str());
        errNo = e.getErrorNo();
        errStr = e.getError();
		exThrown = true;		
	}

	if (exThrown)
	{
		        
        //release modules if some of them have been loaded
        if (index >= 0) {
			pair <auctionProcessListIter_t, auctionProcessListIter_t> ret;
			ret = requests.equal_range(index);	
			for (auctionProcessListIter_t it=ret.first; it!=ret.second; ++it){
				loader->releaseModule((it->second).module);
			}	
		}
		        
    	// Release memory allocated to groups 
		for (splitByModuleIter = splitByModule.begin(); splitByModuleIter != splitByModule.end();  ++splitByModuleIter)
		{
			saveDelete(splitByModuleIter->second);
		}
		
		// Release the used Id
		idSource.freeId(index);
		
        throw Error(errNo, errStr);;
	}

	return index;

#ifdef DEBUG
    log->dlog(ch,"Start addRequest");
#endif

}

/* ------------------------- addAuction ------------------------- */

void AgentProcessor::addAuction( int index, Auction *a, EventScheduler *evs )
{

	bool inserted = false;

    // Verifies that auction given is not null
    if (a == NULL){
		throw Error("The auction given is NULL");
	}

#ifdef DEBUG
    log->dlog(ch,"Start addAuction  set:%s, name:%s to request: %d", 
				a->getSetName().c_str(), a->getAuctionName().c_str(), index);
#endif

    AUTOLOCK(threaded, &maccess);  

	// first search for the index in the requests,  if not found throw an exception.
	auctionProcessListIter_t reqIter = requests.equal_range(index);	
	if (reqIter == requests.end())
	{
		throw Error("Request with index %d not found", index);
	} 
	
	// Get a copy of the parameters
	fieldList_t * parameters = (reqIter->second).parameters;
		
	// After search for the module name on those already loaded.
	sModuleName = a->getAction()->name;

	pair <auctionProcessListIter_t, auctionProcessListIter_t> ret;
	ret = requests.equal_range(index);	
	for (auctionProcessListIter_t it=ret.first; it!=ret.second; ++it){
		if ((it->second).moduleName == sModuleName){
			((it->second).auctions)->push_back(a);
			inserted = true;
			break;
		}
	}
		
	if (inserted == false) {
		requestProcess_t reqProcess;
			
		// load the module
		mod = loader->getModule(sModuleName); 
		reqProcess.module = dynamic_cast<ProcModule*> (mod); 
		if (reqProcess.module != NULL) { // is it a processing kind of module
			reqProcess.mapi = reqProcess.module->getAPI();

			// init module
			reqProcess.index = index;
			reqProcess.parameters = parameters;
			reqProcess.auctions = new auctionDB_t();
			(reqProcess.auctions)->push_back(a);
		}
			
		// Insert in the list of requests needing processing
		requests[index] = reqProcess; 	
	}

#ifdef DEBUG
    log->dlog(ch,"End addAuction");
#endif

}


// add Auctions
void AgentProcessor::addAuctions( int index,  auctionDB_t *auctions, EventScheduler *e )
{

#ifdef DEBUG
    log->dlog(ch,"Start addAuctions");
#endif

    auctionDBIter_t iter;
   
    for (iter = auctions->begin(); iter != auctions->end(); iter++) 
    {
        addAuction(index, *iter, e);
    }

#ifdef DEBUG
    log->dlog(ch,"End addAuctions");
#endif


}

/* ------------------------- delRequest ------------------------- */
void AgentProcessor::delRequest( int index, EventScheduler *e )
{

	// Verifies that the index is valid
	if (index < 0){
		throw ("Invalid index:%d given", index);
	}

    AUTOLOCK(threaded, &maccess);  
	
	pair <auctionProcessListIter_t, auctionProcessListIter_t> ret;
	ret = requests.equal_range(index);	
	for (auctionProcessListIter_t it=ret.first; it!=ret.second; ++it){
		loader->releaseModule((it->second).module);
		saveDelete((it->second).auctions);
	}	
		        
	requests.erase(index);
	// Release the used Id
	idSource.freeId(index);
	
	// TODO AM: call the delete of all associated events.
}


/* ------------------------- delAuction ------------------------- */

void AgentProcessor::delAuction( int index, Auction *a )
{

#ifdef DEBUG
    log->dlog(ch, "deleting Auction #%d", auctionId);
#endif

	// Verifies that the index is valid
	if (index < 0){
		throw ("Invalid index:%d given", index);
	}

    // Verifies that auction given is not null
    if (a == NULL){
		throw Error("The auction given is NULL");
	}

    AUTOLOCK(threaded, &maccess);

	// After search for the module name on those already loaded.
	sModuleName = a->getAction()->name;

	pair <auctionProcessListIter_t, auctionProcessListIter_t> ret;
	ret = requests.equal_range(index);	
	for (auctionProcessListIter_t it=ret.first; it!=ret.second; ++it){
		if ((it->second).moduleName == sModuleName){
			auctionDBIter_t aucIter;
			for (aucIter = (it->second).auctions)->begin(); aucIter != (it->second).auctions)->end(); ++aucIter)
			{
				if ( (aucIter->getSetName() == a->getSetName()) &&
					 (aucIter->getAuctionName()== a->getAuctionName()) ){
					(it->second).auctions)->erase(aucIter);
					break; 
				}
			}
			
			break;
		}
	}
}

// delete auctions
void AgentProcessor::delAuctions(int index,  auctionDB_t *aucts)
{
    auctionDBIter_t iter;

    for (iter = aucts->begin(); iter != aucts->end(); iter++) {
        delAuction(index, *iter);
    }
}


/* ------------------------- execute ------------------------- */

int AgentProcessor::executeRequest( int index, EventScheduler *e )
{

	
	auctionProcess_t *auctionprocess; 
	
    AUTOLOCK(threaded, &maccess);  

	pair <auctionProcessListIter_t, auctionProcessListIter_t> ret;
	ret = requests.equal_range(index);	
	for (auctionProcessListIter_t it=ret.first; it!=ret.second; ++it){
		
		bidDB_t bids;
		
		((it->second).mapi)->execute_user( FieldDefManager::getFieldDefs(), 
										   FieldDefManager::getFieldVals(),
										   (it->second).parameters,
										   (it->second).auctions,
										   &(&bids) );
		
		// Add the Bids create to the local container.
		evnt->addEvent(new AddGeneratedBidsEvent(bids));
				        
	}

    return 0;
}

/* ------------------------- addBid ------------------------- */

void AgentProcessor::addBidAuction( string auctionSet, string auctionName, Bid *b )
{

#ifdef DEBUG
    log->dlog(ch, "adding Bid #%d to auction- Set:%s name:%s", b->getUId(), 
				auctionSet.c_str(), auctionName.c_str());
#endif

    AUTOLOCK(threaded, &maccess);

    auctionProcessListIter_t iter;
    bool found=false;

    for (iter = auctions.begin(); iter != auctions.end(); iter++) 
    {
        Auction *auction = (iter->second).auction; 
        
        if ((auctionSet.compare(auction->getSetName()) == 0) && 
             (auctionName.compare(auction->getAuctionName()) == 0)){
			((iter->second).bids).push_back(b);
			found=true;
		}
    }
	
	if (found==false){
		throw Error("Auction not found: set:%s: name:%s", 
						auctionSet.c_str(), auctionName.c_str());
	}
}


void AgentProcessor::delBids(bidDB_t *bids)
{
	bidDBIter_t bid_iter;   
	for (bid_iter = bids->begin(); bid_iter!= bids->end(); ++bid_iter ){
		Bid *bid = *bid_iter;
		try {
		     delBidAuction(bid->getAuctionSet(), bid->getAuctionName(), bid );
		} catch(Error &err) {
		     log->elog( ch, err.getError().c_str() );
		}
	}
}

/* ------------------------- delBid ------------------------- */

void AgentProcessor::delBidAuction( string auctionSet, string auctionName, Bid *b )
{
    int bidId = b->getUId();

#ifdef DEBUG
    log->dlog(ch, "deleting Bid #%d to auction- Set:%s name:%s", bidId,
			  auctionSet.c_str(), auctionName.c_str());
#endif

    AUTOLOCK(threaded, &maccess);
    
    string bidSet = b->getBidSet();
    string bidName = b->getBidName();
    
    bool deleted=false;
    bool auctionFound= false;

    auctionProcessListIter_t iter;

    for (iter = auctions.begin(); iter != auctions.end(); iter++) 
    {
        Auction *auction = (iter->second).auction; 
        if ((auctionSet.compare(auction->getSetName()) == 0) && 
             (auctionName.compare(auction->getAuctionName()) == 0)){
			auctionFound= true;
			bidDBIter_t bid_iter;
			bid_iter = ((iter->second).bids).begin();
			while (bid_iter != ((iter->second).bids).end()) {
				if ((bidSet.compare((*bid_iter)->getBidSet()) == 0) &&
					(bidName.compare((*bid_iter)->getBidName()) == 0)){
					((iter->second).bids).erase(bid_iter);
					deleted=true;
				}
				++bid_iter;
			} 
		}
    }
	
	if (auctionFound==false){
		throw Error("Auction not found: set:%s: name:%s", 
						auctionSet.c_str(), auctionName.c_str());
	}

	if (deleted==false){
		throw Error("Bid not found: set:%s: name:%s", 
						bidSet.c_str(), bidName.c_str());
	}
}



int AgentProcessor::handleFDEvent(eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds)
{

    return 0;
}

void AgentProcessor::main()
{

    // this function will be run as a single thread inside the Agent processor
    log->log(ch, "AUM Processor thread running");
    
    for (;;) {
        handleFDEvent(NULL, NULL,NULL, NULL);
    }
}       

void AgentProcessor::waitUntilDone(void)
{
#ifdef ENABLE_THREADS
    AUTOLOCK(threaded, &maccess);

    if (threaded) {
      while (queue->getUsedBuffers() > 0) {
        threadCondWait(&doneCond, &maccess);
      }
    }
#endif
}


string AgentProcessor::getInfo()
{
    ostringstream s;

    AUTOLOCK(threaded, &maccess);

    s << "Agent processor";

    return s.str();
}



/* -------------------- addTimerEvents -------------------- */

void AgentProcessor::addTimerEvents( int bidID, int actID,
                                      EventScheduler &es )
{
    /* TODO AM : Review if this code is required.
    timers_t *timers = (act.mapi)->getTimers(act.flowData);

    if (timers != NULL) {
        while (timers->flags != TM_END) {
            es.addEvent(new ProcTimerEvent(bidID, actID, timers++));
        }
    }
    */ 
}

// handle module timeouts
void AgentProcessor::timeout(int rid, int actid, unsigned int tmID)
{
    /* TODO AM : Review if this code is required.
    ppaction_t *a;
    ruleActions_t *ra;

    AUTOLOCK(threaded, &maccess);

    ra = &rules[rid];

    a = &ra->actions[actid];
    a->mapi->timeout(tmID, a->flowData);
    */
}


/* ------------------------- dump ------------------------- */

void AgentProcessor::dump( ostream &os )
{

    os << "AUM Processor dump :" << endl;
    os << getInfo() << endl;

}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, AgentProcessor &pe )
{
    pe.dump(os);
    return os;
}
