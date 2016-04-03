
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
#include "stdincpp.h"
#include "AgentProcessor.h"
#include "ProcError.h"

using namespace auction;

/* ------------------------- AUMProcessor ------------------------- */

AgentProcessor::AgentProcessor(int domain, ConfigManager *cnf, string fdname, string fvname, int threaded, string moduleDir ) 
    : AuctionManagerComponent(cnf, "AGENT_PROCESSOR", threaded), 
	  FieldDefManager(fdname, fvname), idSource(0), domain(domain)
{
    string txt;
    
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

    if (moduleDir.empty()) {
		txt = cnf->getValue("ModuleDir", "AGENT_PROCESSOR");
        if ( txt != "") {
            moduleDir = txt;
        }        
    }

    try {
        loader = new ModuleLoader(cnf, 
								  moduleDir.c_str() /*module (lib) basedir*/,
                                  cnf->getValue("Modules", "AGENT_PROCESSOR"),/*modlist*/
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
int AgentProcessor::addRequest( string sessionId, fieldList_t *parameters, Auction *auction, time_t start, time_t stop )
{

#ifdef DEBUG
    log->dlog(ch,"Start addRequest");
#endif

	Module *mod;
	string sModuleName;
	int index = -2;
    bool exThrown = false;	
    int errNo; 
    string errStr;

	// Verifies that auction must be different from null
	if (auction == NULL){
		throw Error("Auction pointer given for add request function is NULL");
	}

	// Verifies that parameters must be different from null
	if (parameters == NULL){
		throw Error("parameters pointer given for add request function is NULL");
	}

    AUTOLOCK(threaded, &maccess);  


	try {
				
		// Read the name of the module to load
		sModuleName = auction->getModuleName();
		sModuleName = sModuleName + "user";
		requestProcess reqProcess;
					
		// load the module
		mod = loader->getModule(sModuleName); 
		reqProcess.setModule( dynamic_cast<ProcModule*> (mod)); 
		if (reqProcess.getModule() != NULL) { // is it a processing kind of module
			reqProcess.setProcessModuleInterface(reqProcess.getModule()->getAPI());

			// init module
			index = idSource.newId();
			reqProcess.setSession(sessionId);
			reqProcess.setUId(index);
			reqProcess.setModuleName(sModuleName);

			reqProcess.setParameters(parameters);
			reqProcess.insertAuction(auction);
			reqProcess.setStart(start);
			reqProcess.setStop(stop);
		}
			
		// Insert in the list of requests needing processing
		requests[index] = reqProcess;
				  
    } catch (Error &e) { 
		
        log->elog(ch, e);
        errNo = e.getErrorNo();
        errStr = e.getError();
		exThrown = true;	
		
    } catch (ProcError &e) {
		
        log->elog(ch, e.getError().c_str());
        errNo = e.getErrorNo();
        errStr = e.getError();
		exThrown = true;		
		
	}

	if (exThrown)
	{
		        
        //release modules if some of them have been loaded
        if (index >= 0) {
			requestProcessListIter_t ret;
			ret = requests.find(index);	
			if (ret != requests.end()){
				loader->releaseModule((ret->second).getModule());
			}	
			
			// Release the used Id
			idSource.freeId(index);
		}
		        				
        throw Error(errNo, errStr);;
	}

#ifdef DEBUG
    log->dlog(ch,"ending addRequest");
#endif

	return index;

}

/* ------------------------- delRequest ------------------------- */
void AgentProcessor::releaseRequest( int index )
{

#ifdef DEBUG
    log->dlog(ch,"starting releaseRequest");
#endif


	// Verifies that the index is valid
	if (index < 0){
		throw Error("Invalid index:%d given", index);
	}

    AUTOLOCK(threaded, &maccess);  
	
	requestProcessListIter_t ret = requests.find(index);	
	if (ret != requests.end()){
		loader->releaseModule((ret->second).getModule());
				
	} else {
		throw Error("Request index:%d not found", index);
	}

#ifdef DEBUG
    log->dlog(ch,"ending releaseRequest");
#endif
		        	
}



/* ------------------------- delRequest ------------------------- */
void AgentProcessor::delRequest( int index )
{

#ifdef DEBUG
    log->dlog(ch,"starting delRequest");
#endif


	// Verifies that the index is valid
	if (index < 0){
		throw Error("Invalid index:%d given", index);
	}

    AUTOLOCK(threaded, &maccess);  
	
	requestProcessListIter_t ret = requests.find(index);	
	if (ret != requests.end()){
		
		loader->releaseModule((ret->second).getModule());
		
		requests.erase(ret);		
		// Release the used Id
		idSource.freeId(index);
	} else {
		
		log->dlog(ch, "Request index:%d not found", index);
	}

#ifdef DEBUG
    log->dlog(ch,"ending delRequest");
#endif
		        	
}

/* ------------------------- executeRequest  ------------------------- */
void  
AgentProcessor::executeRequest( int index, EventScheduler *e )
{

#ifdef DEBUG
    log->dlog(ch,"starting executeRequest");
#endif

	
    AUTOLOCK(threaded, &maccess);  

	requestProcessListIter_t ret;
	ret = requests.find(index);	
	if (ret != requests.end()){
		
		biddingObjectDB_t bids;
		biddingObjectDB_t *ptr = &bids;
		
		try{ 

			configParam_t *params = new configParam_t[2];
			int i = 0;
		
			string paramName2 = "domainid";
			string paramValue2 = getDomainStr();
			if (!paramValue2.empty()){
				params[i].name = (char* ) paramName2.c_str();
				params[i].value = (char *) paramValue2.c_str();
				i++;
			
				params[i].name = NULL;
				params[i].value = NULL;
			
				((ret->second).getMAPI())->reset( params );

			}
			
			((ret->second).getMAPI())->execute_user( FieldDefManager::getFieldDefs(), 
										   FieldDefManager::getFieldVals(),
										   (ret->second).getParameters(),
										   (ret->second).getAuctions(),
										   (ret->second).getStart(),
										   (ret->second).getStop(),
										   &ptr );
			
			delete[] params;
			
		} catch (ProcError &e){
			log->elog(ch,e.getError().c_str());
			throw Error(e.getError().c_str());
		}

#ifdef DEBUG
		biddingObjectDBIter_t        iter;
		for (iter = bids.begin(); iter != bids.end();++iter) {
			BiddingObject *b = (*iter);
			if (b != NULL){
				log->dlog(ch, "New BiddingObject After Push Execution: %s.%s", b->getBiddingObjectSet().c_str(), 
					b->getBiddingObjectName().c_str());
			}	
		}	
#endif

		// Add the Bids (bidding objects) created to the local container.
		e->addEvent(new AddGeneratedBiddingObjectsEvent(index, bids));


		
	} else {
		throw Error("Request with index:%d was not found", index);
	}

#ifdef DEBUG
    log->dlog(ch,"ending executeRequest");
#endif


}

/* ------------------------- addAuction ------------------------- */
void 
AgentProcessor::addAuctionRequest( int index, Auction *a )
{

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
	requestProcessListIter_t reqIter = requests.find(index);	
	if (reqIter == requests.end())
	{
		throw Error("Request with index %d not found", index);
	} 
	
	// Second look the auction in the list of auction already inserted, if it exists generates an error
	reqIter = requests.find(index);	
	auctionDBIter_t iterAuct;
	for (iterAuct = ((reqIter->second).auctions).begin(); iterAuct != ((reqIter->second).auctions).end(); ++iterAuct)
	{
		Auction *aTmp = *iterAuct;
		if ( (aTmp->getSetName() == a->getSetName()) &&
			  (aTmp->getAuctionName() == a->getAuctionName()) ) {
			throw Error("Auction %s.%s is inserted in the request process with index %d not found", 
							a->getSetName().c_str(), a->getAuctionName().c_str(),  index);  
		} 
	}
			
	// After search for the module name on those already loaded.
	string sModuleName = a->getAction()->name;
	sModuleName = sModuleName + "user";

	if ((reqIter->second).getModuleName() == sModuleName){
			(reqIter->second).insertAuction(a);
	} else {
		throw Error("The module to load:%s must be the same: %s for the request %d", 
						sModuleName.c_str(), (reqIter->second).getModuleName().c_str(),  index);
	}
		
#ifdef DEBUG
    log->dlog(ch,"End addAuction");
#endif

}

/* ------------------------- addAuctions ------------------------- */
void 
AgentProcessor::addAuctionsRequest( int index,  auctionDB_t *auctions )
{

#ifdef DEBUG
    log->dlog(ch,"Start add Auctions To Request");
#endif

    auctionDBIter_t iter;
    for (iter = auctions->begin(); iter != auctions->end(); iter++) 
    {
        addAuctionRequest(index, *iter);
    }

#ifdef DEBUG
    log->dlog(ch,"End add Auctions to Request");
#endif

}


/* ------------------------- delAuction ------------------------- */
void 
AgentProcessor::delAuctionRequest( int index, Auction *a )
{

#ifdef DEBUG
    log->dlog(ch, "deleting Auction #%d", a->getUId());
#endif

	// Verifies that the index is valid
	if (index < 0){
		throw Error("Invalid index:%d given", index);
	}

    // Verifies that auction given is not null
    if (a == NULL){
		throw Error("The auction given is NULL");
	}

    AUTOLOCK(threaded, &maccess);

	requestProcessListIter_t ret;
	ret = requests.find(index);	
	if (ret != requests.end()) {

		auctionDBIter_t aucIter;
		for (aucIter = ((ret->second).getAuctions())->begin(); aucIter != ((ret->second).getAuctions())->end(); ++aucIter)
		{
			if ( ((*aucIter)->getSetName() == a->getSetName()) &&
				 ((*aucIter)->getAuctionName()== a->getAuctionName()) ) {
				((ret->second).getAuctions())->erase(aucIter);
				break; 
			}
		}
	} else {
		throw Error("Auction %s.%s not found in request %d", 
						a->getSetName().c_str(), a->getAuctionName().c_str(), index );
	}
}


/* ------------------------- delAuctions ------------------------- */
void 
AgentProcessor::delAuctionsRequest(int index,  auctionDB_t *aucts)
{
	
    auctionDBIter_t iter;
    for (iter = aucts->begin(); iter != aucts->end(); iter++) {
        delAuctionRequest(index, *iter);
    }
}

void 
AgentProcessor::delAuctions(auctionDB_t *aucts)
{
	AUTOLOCK(threaded, &maccess);
	
    requestProcessListIter_t iter = requests.begin();
    for (; iter != requests.end(); ++iter){
		// delete the actions from the request process.
 		delAuctionsRequest(iter->first, aucts);		
	}
	
	
	// If the number of auction left in the request is zero, then delete the request process.
	for (iter = requests.begin() ; iter != requests.end();){
        requestProcessListIter_t iter2 = requests.find(iter->first);
        if ((iter2->second).getAuctions()->size() == 0){
			releaseRequest( iter->first );
			// Release the used Id
			idSource.freeId(iter->first);
 
			requests.erase(iter++);
		} 
		else {
			++iter;
		}
	}
}

string 
AgentProcessor::getSession(int index)
{
	AUTOLOCK(threaded, &maccess);
	
    requestProcessListIter_t iter;
	iter = requests.find(index);
	if (iter != requests.end()){
		return (iter->second).getSession();
	} else {
		throw Error("Request process with id:%d not found", index);
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
		// Not implemented.
    }
#endif
}

string 
AgentProcessor::getDomainStr()
{

    ostringstream s;

    AUTOLOCK(threaded, &maccess);

    s << domain;

    return s.str();

}


string 
AgentProcessor::getInfo()
{
    ostringstream s;

    AUTOLOCK(threaded, &maccess);

    s << "Agent processor";

    return s.str();
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
