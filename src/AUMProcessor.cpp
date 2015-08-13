
/*!\file   AUMProcessor.cpp

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
		manages and applies auction processing modules

    $Id: AUMProcessor.cpp 748 2015-07-23 14:33:00Z amarentes $
*/

#include <stdio.h>
#include <stdlib.h>

#include "ProcError.h"
#include "AUMProcessor.h"
#include "Module.h"
#include "ParserFcts.h"



/* ------------------------- AUMProcessor ------------------------- */

AUMProcessor::AUMProcessor(ConfigManager *cnf, int threaded, string moduleDir ) 
    : AuctionManagerComponent(cnf, "AUM_PROCESSOR", threaded)
{
    string txt;
    
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

    if (moduleDir.empty()) {
		txt = cnf->getValue("ModuleDir", "AUM_PROCESSOR");
        if ( txt != "") {
            moduleDir = txt;
        }        
    }

    try {
        loader = new ModuleLoader(cnf, moduleDir.c_str() /*module (lib) basedir*/,
                                  cnf->getValue("Modules", "AUM_PROCESSOR"),/*modlist*/
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

AUMProcessor::~AUMProcessor()
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
		
    // discard the Module Loader
    saveDelete(loader);

}

// add bids
void AUMProcessor::addBids( bidDB_t *bids, EventScheduler *e )
{
    bidDBIter_t iter;
   
    for (iter = bids->begin(); iter != bids->end(); iter++) {
        addBid(*iter, e);
    }
}

// add Auctions
void AUMProcessor::addAuctions( auctionDB_t *auctions, EventScheduler *e )
{

#ifdef DEBUG
    log->dlog(ch,"Start addAuctions");
#endif

    auctionDBIter_t iter;
   
    for (iter = auctions->begin(); iter != auctions->end(); iter++) 
    {
        addAuction(*iter, e);
    }

#ifdef DEBUG
    log->dlog(ch,"End addAuctions");
#endif


}



// add bids
void AUMProcessor::addBids( bidDB_t *bids )
{

}


// delete bids
void AUMProcessor::delBids(bidDB_t *bids)
{
    bidDBIter_t iter;

    for (iter = bids->begin(); iter != bids->end(); iter++) {
        delBid(*iter);
    }
}

// delete auctions
void AUMProcessor::delAuctions(auctionDB_t *aucts)
{
    auctionDBIter_t iter;

    for (iter = aucts->begin(); iter != aucts->end(); iter++) {
        delAuction(*iter);
    }
}


/* ------------------------- execute ------------------------- */

int AUMProcessor::executeAuction(int rid, string rname)
{

	auctionProcess_t *auctionprocess; 
	
    AUTOLOCK(threaded, &maccess);  

    auctionprocess = &auctions[rid];
	(auctionprocess->action.mapi)->execute( (auctionprocess->action).params, (void* *) &(auctionprocess->bids) );
	
    return 0;
}

/* ------------------------- addBid ------------------------- */

int AUMProcessor::addBid( Bid *b, EventScheduler *e )
{

}

/* ------------------------- addAuction ------------------------- */

int AUMProcessor::addAuction( Auction *a, EventScheduler *evs )
{

#ifdef DEBUG
    log->dlog(ch,"Start addAuction");
#endif
   
    auctionProcess_t entry;
    int errNo;
    string errStr;
    bool exThrown = false;

    int auctionId = a->getUId();
    action_t *action = a->getAction();

#ifdef DEBUG
    log->dlog(ch, "Adding auction #%d", auctionId);
#endif  

    AUTOLOCK(threaded, &maccess);  

    entry.action.module = NULL;
    entry.action.params = NULL;

    Module *mod;
    string mname = action->name;

#ifdef DEBUG
    log->dlog(ch, "It is going to load module %s", mname.c_str());
#endif 

    try{        	    
		// load Action Module used by this rule
		mod = loader->getModule(mname.c_str());
		entry.action.module = dynamic_cast<ProcModule*> (mod);

		if (entry.action.module != NULL) { // is it a processing kind of module

#ifdef DEBUG
    log->dlog(ch, "module %s loaded", mname.c_str());
#endif 
			 entry.action.mapi = entry.action.module->getAPI();

			 // init module
			 configItemList_t itmConf = action->conf;
			
            // add timer events once (only adds events on first module use)
            entry.action.module->addTimerEvents( *evs );
		
		}
		// make sure the vector of rules is large enough
		if ((unsigned int)auctionId + 1 > auctions.size()) {
			auctions.reserve( auctionId*2 + 1);
			auctions.resize(auctionId + 1 );
		}
		// success ->enter struct into internal table
		auctions[auctionId] = entry;

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
        log->elog(ch, e);
        errNo = e.getErrorNo();
        errStr = e.getError();
		exThrown = true;		
	}

	if (exThrown)
	{
    
        //release packet processing modules already loaded for this rule
        if (entry.action.module) {
            loader->releaseModule(entry.action.module);
        }

        throw Error(errNo, errStr);;
	}

#ifdef DEBUG
    log->dlog(ch,"End addAuction");
#endif
    
    return 0;

}


/* ------------------------- delBid ------------------------- */

int AUMProcessor::delBid( Bid *b )
{
    int bidId = b->getUId();

#ifdef DEBUG
    log->dlog(ch, "deleting Bid #%d", bidId);
#endif

    AUTOLOCK(threaded, &maccess);

	// TODO AM: implement this procedure.

    return 0;
}


/* ------------------------- delBid ------------------------- */

int AUMProcessor::delAuction( Auction *a )
{
    auctionProcess_t *entry;
    int auctionId; 

#ifdef DEBUG
    log->dlog(ch, "deleting Auction #%d", auctionId);
#endif

    AUTOLOCK(threaded, &maccess);

    auctionId = a->getUId();
        
    entry = &auctions[auctionId];
    
    // TODO AM: dismantle auction data structure with module function
    //(entry->mapi)->destroyExportRec(i->expData);
        
    // release modules loaded for this rule
    loader->releaseModule((entry->action).module);
       
    return 0;
}


int AUMProcessor::handleFDEvent(eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds)
{

    return 0;
}

void AUMProcessor::main()
{

    // this function will be run as a single thread inside the AUM processor
    log->log(ch, "AUM Processor thread running");
    
    for (;;) {
        handleFDEvent(NULL, NULL,NULL, NULL);
    }
}       

void AUMProcessor::waitUntilDone(void)
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


string AUMProcessor::getInfo()
{
    ostringstream s;

    AUTOLOCK(threaded, &maccess);

    s << loader->getInfo();  // get the list of loaded modules

    return s.str();
}



/* -------------------- addTimerEvents -------------------- */

void AUMProcessor::addTimerEvents( int bidID, int actID,
                                      ppaction_t &act, EventScheduler &es )
{
    /*
    timers_t *timers = (act.mapi)->getTimers(act.flowData);

    if (timers != NULL) {
        while (timers->flags != TM_END) {
            es.addEvent(new ProcTimerEvent(bidID, actID, timers++));
        }
    }
    */ 
}

// handle module timeouts
void AUMProcessor::timeout(int rid, int actid, unsigned int tmID)
{
    /*
    ppaction_t *a;
    ruleActions_t *ra;

    AUTOLOCK(threaded, &maccess);

    ra = &rules[rid];

    a = &ra->actions[actid];
    a->mapi->timeout(tmID, a->flowData);
    */
}

/* -------------------- getModuleInfoXML -------------------- */

string AUMProcessor::getModuleInfoXML( string modname )
{
    AUTOLOCK(threaded, &maccess);
    return loader->getModuleInfoXML( modname );
}


/* ------------------------- dump ------------------------- */

void AUMProcessor::dump( ostream &os )
{

    os << "AUM Processor dump :" << endl;
    os << getInfo() << endl;

}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, AUMProcessor &pe )
{
    pe.dump(os);
    return os;
}
