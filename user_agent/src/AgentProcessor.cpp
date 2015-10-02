
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

AgentProcessor::AgentProcessor(ConfigManager *cnf, string fdname, int threaded ) 
    : AuctionManagerComponent(cnf, "AGENT_PROCESSOR", threaded), fieldDefFileName(fdname)
{
    string txt;
    
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

	loadFieldDefs(fdname);

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

/* -------------------- isReadableFile -------------------- */

static int isReadableFile( string fileName ) {

    FILE *fp = fopen(fileName.c_str(), "r");

    if (fp != NULL) {
        fclose(fp);
        return 1;
    } else {
        return 0;
    }
}

/* -------------------- loadFieldDefs -------------------- */

void AgentProcessor::loadFieldDefs(string fname)
{
    if (fieldDefFileName.empty()) {
        if (fname.empty()) {
            fname = FIELDDEF_FILE;
		}
    } else {
        fname = fieldDefFileName;
    }

#ifdef DEBUG
    log->dlog(ch, "filename %s", fname.c_str());
#endif

    if (isReadableFile(fname)) {
        if (fieldDefs.empty() && !fname.empty()) {
            FieldDefParser f = FieldDefParser(fname.c_str());
            f.parse(&fieldDefs);
        }
    
    }else{
#ifdef DEBUG
    log->dlog(ch, "filename %s is not readable", fname.c_str());
#endif    
    }
    
}


// add Auctions
void AgentProcessor::addAuctions( auctionDB_t *auctions, EventScheduler *e )
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

// delete auctions
void AgentProcessor::delAuctions(auctionDB_t *aucts)
{
    auctionDBIter_t iter;

    for (iter = aucts->begin(); iter != aucts->end(); iter++) {
        delAuction(*iter);
    }
}


/* ------------------------- execute ------------------------- */

int AgentProcessor::executeAuction(int rid, string rname)
{

	// TODO AM: Code to implement.
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

/* ------------------------- addAuction ------------------------- */

int AgentProcessor::addAuction( Auction *a, EventScheduler *evs )
{

#ifdef DEBUG
    log->dlog(ch,"Start addAuction");
#endif
   
    auctionProcess_t entry;
    string errStr;

    int auctionId = a->getUId();

#ifdef DEBUG
    log->dlog(ch, "Adding auction #%d - set:%s, name:%s", 
				 auctionId, a->getSetName().c_str(), a->getAuctionName().c_str());
#endif  

    AUTOLOCK(threaded, &maccess);  

	entry.auction = a;
	auctions[auctionId] = entry;


#ifdef DEBUG
    log->dlog(ch,"End addAuction");
#endif
    
    return 0;

}


void AgentProcessor::delBids(bidDB_t *bids)
{
	bidDBIter_t bid_iter;   
	for (bid_iter = bids->begin(); bid_iter!= bids->end(); ++bid_iter ){
		Bid *bid = *bid_iter;
		bidAuctionListIter_t auction_iter;
		for (auction_iter = (bid->getAuctions())->begin(); 
			 auction_iter!= (bid->getAuctions())->end(); ++auction_iter){
			 try{
			     delBidAuction((auction_iter->second).auctionSet, 
							   (auction_iter->second).auctionName, bid );
			 } catch(Error &err){
				  log->elog( ch, err.getError().c_str() );
			 }
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
    
    string bidSet = b->getSetName();
    string bidName = b-> getBidName();
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
				if ((bidSet.compare((*bid_iter)->getSetName()) == 0) &&
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


/* ------------------------- delAuction ------------------------- */

int AgentProcessor::delAuction( Auction *a )
{
    auctionProcess_t *entry;
    int auctionId; 

#ifdef DEBUG
    log->dlog(ch, "deleting Auction #%d", auctionId);
#endif

    AUTOLOCK(threaded, &maccess);

    auctionId = a->getUId();
        
    entry = &auctions[auctionId];
                   
    return 0;
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
