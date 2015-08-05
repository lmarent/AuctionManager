
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
    auctionDBIter_t iter;
   
    for (iter = auctions->begin(); iter != auctions->end(); iter++) {
        addAuction(*iter, e);
    }
}



// add bids
void AUMProcessor::addBids( bidDB_t *bids )
{

}

// add auctions
void AUMProcessor::addAuctions( auctionDB_t *auctions )
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
void AUMProcessor::delAuctions(auctionDB_t *auctions)
{
    auctionDBIter_t iter;

    for (iter = auctions->begin(); iter != auctions->end(); iter++) {
        delAuction(*iter);
    }
}


/* ------------------------- execute ------------------------- */

int AUMProcessor::execute( EventScheduler *e )
{
    int bidId;
    
    /* TODO AM: to implement.

    ruleActions_t entry;
    actionList_t *actions;
    int errNo;
    string errStr;
    bool exThrown = false;

    ruleId  = r->getUId();
    actions = r->getActions();

#ifdef DEBUG
    log->dlog(ch, "adding Rule #%d", ruleId);
#endif  

    AUTOLOCK(threaded, &maccess);  

    entry.lastPkt = 0;
    entry.packets = 0;
    entry.bytes = 0;
    entry.flist = r->getFilter();
    entry.bidir = r->isBidir();
    entry.seppaths = r->sepPaths();
    entry.rule = r;

    //  Load the module.
    ppaction_t a;

    a.module = NULL;
    a.params = NULL;
    a.flowData = NULL;

    Module *mod;
    string mname = iter->name;

#ifdef DEBUG
    log->dlog(ch, "it is going to load module %s", mname.c_str());
#endif 

    try{        	    
		// load Action Module used by this rule
		mod = loader->getModule(mname.c_str());
		a.module = dynamic_cast<ProcModule*> (mod);

		if (a.module != NULL) { // is it a processing kind of module

#ifdef DEBUG
    log->dlog(ch, "module %s loaded", mname.c_str());
#endif 
			 a.mapi = a.module->getAPI();

			 // init module
			 configItemList_t itmConf = iter->conf;
					
			// init timers
			addTimerEvents(ruleId, cnt, a, *e);
		
			entry.actions.push_back(a);

		}	
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
        if (i->module) {
            loader->releaseModule(i->module);
        }

        // empty the list itself
        entry.actions.clear();

        throw Error(errNo, errStr);;
	}
	*/
    return 0;
}

/* ------------------------- addBid ------------------------- */

int AUMProcessor::addBid( Bid *b, EventScheduler *e )
{

}

/* ------------------------- addAuction ------------------------- */

int AUMProcessor::addAuction( Auction *a, EventScheduler *e )
{

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
    int auctionId = a->getUId();

#ifdef DEBUG
    log->dlog(ch, "deleting Auction #%d", auctionId);
#endif

    AUTOLOCK(threaded, &maccess);

	// TODO AM: implement this procedure.

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


/* ------------------------- AuctionTimeout ------------------------- */

// return 0 (if timeout), 1 (stays idle), >1 (active and no timeout yet)
unsigned long AUMProcessor::auctionTimeout(int auctionID, unsigned long ival, time_t now)
{
    AUTOLOCK(threaded, &maccess);
	/*
    time_t last = bids[bidID].lastPkt;

    if (last > 0) {
        //ruleActions_t *ra = &bids[bidID];
		 log->dlog(ch,"auto flow idle, export: YES");
         return 0;
    }
	*/
    return 1;
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
