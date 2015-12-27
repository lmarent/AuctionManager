
/*!\file   AnslpProcessor.cpp

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
		receive and create events for objects returned from the anslp module.

    $Id: AUMProcessor.cpp 748 2015-07-23 14:33:00Z amarentes $
*/



#include "ParserFcts.h"
#include "AnslpProcessor.h"
#include "benchmark_journal.h"
#include "EventAuctioner.h"

#ifdef BENCHMARK
  extern benchmark_journal journal;
#endif

using namespace anslp;
using namespace anslp::msg;
using namespace auction;

AnslpProcessor::AnslpProcessor(ConfigManager *cnf, int threaded ) 
    : AuctionManagerComponent(cnf, "ANSLP_PROCESSOR", threaded)
{
#ifdef DEBUG
    log->dlog(ch,"Starting ANSLP Processor");
#endif
	
	FastQueue *_queue = new anslp::FastQueue();
	queue = _queue;
	
	assert(_queue != NULL);
}

AnslpProcessor::~AnslpProcessor()
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
	
	saveDelete(queue);	
}

void AnslpProcessor::process(eventVec_t *e, AnslpEvent *evt)
{

#ifdef DEBUG
    log->dlog(ch,"Starting ANSLP Processor - process");
#endif

	assert( evt != NULL );
	
	string sessionId;
	
	if ( is_check_event(evt) ) {
		CheckEvent *che =
			dynamic_cast<CheckEvent *>(evt);
				
		sessionId = che->getSession();

		auction::CreateCheckSessionEvent *retEvent = new CreateCheckSessionEvent(sessionId, che->getQueue());
				
		anslp::objectList_t *objects = che->getObjects();
		anslp::objectListIter_t it;
		for (it = objects->begin(); it != objects->end(); ++it){
			retEvent->setObject(it->first, it->second->copy());
		}
		
		e->push_back(retEvent);	
				
		return;
	}

	if ( is_addsession_event(evt) ) {
		anslp::AddSessionEvent *ase =
			dynamic_cast<AddSessionEvent *>(evt);

		string sessionId = ase->getSession();
		CreateSessionEvent *retEvent = new CreateSessionEvent(sessionId, ase->getQueue());
		
		anslp::objectList_t *objects = ase->getObjects();
		anslp::objectListIter_t it;
		for (it = objects->begin(); it != objects->end(); ++it){
			retEvent->setObject(it->first,it->second->copy());
		}
		
		e->push_back(retEvent);	
		
		return;
	}

	if ( is_response_addsession_event(evt) ) {
		anslp::ResponseAddSessionEvent *ras =
			dynamic_cast<ResponseAddSessionEvent *>(evt);
		
		// This message type must not be comming, log the error.
		log->elog(ch,"Received message response add session which is not expected");
		return;
	}	

	if ( is_response_checksession_event(evt) ) {
		anslp::ResponseCheckSessionEvent *rcs =
			dynamic_cast<ResponseCheckSessionEvent *>(evt);
			
		// This message type must not be comming, log the error.
		log->elog(ch,"Received message response check session which is not expected");									
		return;
	}

	if ( is_auction_interaction_event(evt) ) {
		anslp::AuctionInteractionEvent *aie =
			dynamic_cast<anslp::AuctionInteractionEvent *>(evt);

		string sessionId = aie->getSession();
		AuctionInteractionEvent *retEvent = new AuctionInteractionEvent(sessionId);

		assert(retEvent != NULL);

		anslp::objectList_t *objects = aie->getObjects();
		anslp::objectListIter_t it;
		for (it = objects->begin(); it != objects->end(); ++it){
			
#ifdef DEBUG
    log->dlog(ch,"objectId: %s", (it->first).to_string().c_str());
#endif		
			assert(it->second != NULL);

#ifdef DEBUG
    log->dlog(ch,"after assert");
#endif		
			retEvent->setObject(it->first, it->second->copy());
		}

		e->push_back(retEvent);	
		
		return;
	}

#ifdef DEBUG
    log->dlog(ch,"Ending ANSLP Processor - process");
#endif


}

int 
AnslpProcessor::handleFDEvent(eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds)
{

#ifdef DEBUG
    log->dlog(ch,"Starting ANSLP Processor handleFDEvent");
#endif
	
	assert( e != NULL );

	// Takes from the queue.
	anslp::FastQueue *anslp_input = get_fqueue();

	// A timeout makes sure the loop condition is checked regularly.
	anslp::AnslpEvent *evt = anslp_input->dequeue_timedwait(1000);
		
	if ( evt == NULL ){
		return 0;	// no message in the queue
	}

#ifdef DEBUG
    log->dlog(ch,"ANSLP Processor: message in queue");
#endif
			
	MP(benchmark_journal::PRE_PROCESSING);

	// Then feed the event to the dispatcher.
	if ( evt != NULL ) {
		MP(benchmark_journal::PRE_DISPATCHER);
		process(e, evt);
		MP(benchmark_journal::POST_DISPATCHER);
		delete evt;
	}

	MP(benchmark_journal::POST_PROCESSING);

#ifdef DEBUG
    log->dlog(ch,"ending ANSLP Processor handleFDEvent");
#endif

	
	return 0;
		
}

void 
AnslpProcessor::waitUntilDone(void)
{
#ifdef ENABLE_THREADS
    AUTOLOCK(threaded, &maccess);
	// TODO AM: Implement.

#endif
}


// TODO AM : Implement threads.
void AnslpProcessor::main()
{
    // this function will be run as a single thread inside the anslp processor
    log->log(ch, "anslp thread running");
    
    for (;;) {
        handleFDEvent(NULL, NULL,NULL, NULL);
    }

}

string 
AnslpProcessor::getInfo()
{
    ostringstream s;

    AUTOLOCK(threaded, &maccess);

    s << "Get Info" << endl;

    return s.str();
}


/* ------------------------- dump ------------------------- */

void AnslpProcessor::dump( ostream &os )
{

    os << "AnslpProcessor dump :" << endl;
    os << getInfo() << endl;

}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, AnslpProcessor &p )
{
    p.dump(os);
    return os;
}
