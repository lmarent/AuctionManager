
/*!\file   EventScheduler.cc

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
    Code based on Netmate Implementation

    $Id: EventScheduler.h 748 2015-07-30 19:05:00 amarentes $
*/

#include "ParserFcts.h"
#include "Error.h"
#include "EventScheduler.h"
#include "Timeval.h"

using namespace auction;

// for reading from trace files that have time gaps: make sure that
// event is not in the past
// this will cause all events to be 'suspended' during the time gap
#define TIME_GAP_HACK 0


// min timeout for select() in us (10ms minimum on current UNIX!)
const int MIN_TIMEOUT = 10000;


/* ------------------------- EventScheduler ------------------------- */

EventScheduler::EventScheduler() 
{

    log = Logger::getInstance();
    ch = log->createChannel("EventScheduler");
#ifdef DEBUG
    log->dlog(ch, "Starting");
#endif
}


/* ------------------------- ~EventScheduler ------------------------- */

EventScheduler::~EventScheduler()
{
    eventListIter_t iter;

#ifdef DEBUG
    log->dlog(ch, "Shuting down Event Scheduler");
#endif

    // free all stored events
    for (iter = events.begin(); iter != events.end(); iter++) {
        saveDelete(iter->second);
    }
    
#ifdef DEBUG
    log->dlog(ch, "Shutdown Event Scheduler");
#endif
    
}


/* ------------------------- addEvent ------------------------- */

void EventScheduler::addEvent(Event *ev)
{
	
	
#ifdef DEBUG
	struct timeval tv = ev->getTime();
    log->dlog(ch,"new event %s - time: %s", eventNames[ev->getType()].c_str(), (Timeval::toString(tv)).c_str());
#endif
    
    events.insert(make_pair(ev->getTime(),ev));

/*    
#ifdef DEBUG
    log->dlog(ch,"nbr of events active after inserting:", events.size());
    eventListIter_t iter;
            
    // output all scheduled Events to ostream
    for (iter = events.begin(); iter != events.end(); iter++) {
        struct timeval rv = iter->first;
        log->dlog(ch,"Time:%s event:%s", (Timeval::toString(rv)).c_str(), 
					eventNames[iter->second->getType()].c_str());        
    }
    
#endif
*/
    
}


void EventScheduler::delBiddingObjectEvents(int uid)
{
    int ret = 0;
    eventListIter_t iter, tmp;

    // search linearly through list for bid with given ID and delete entries
    iter = events.begin();
    while (iter != events.end()) {
        tmp = iter;
        iter++;
        
        ret = tmp->second->deleteBiddingObject(uid);
        if (ret == 1) {
            // ret = 1 means bidding object was present in event but other bidding objects are still in
            // the event
#ifdef DEBUG
            log->dlog(ch,"remove bid %d from event %s", uid, 
                      eventNames[tmp->second->getType()].c_str());
#endif
        } else if (ret == 2) {
            // ret=2 means the event is now empty and therefore can be deleted
#ifdef DEBUG
            log->dlog(ch,"remove event %s", eventNames[tmp->second->getType()].c_str());
#endif
           
            saveDelete(tmp->second);
            events.erase(tmp);
        } 
    }
}


void EventScheduler::delAuctionEvents(int uid)
{
    int ret = 0;
    eventListIter_t iter, tmp;

    // search linearly through list for bid with given ID and delete entries
    iter = events.begin();
    while (iter != events.end()) {
        tmp = iter;
        iter++;
        
        ret = tmp->second->deleteAuction(uid);
        if (ret == 1) {
            // ret = 1 means rule was present in event but other auctions are still in
            // the event
#ifdef DEBUG
            log->dlog(ch,"remove auction %d from event %s", uid, 
                      eventNames[tmp->second->getType()].c_str());
#endif
        } else if (ret == 2) {
            // ret=2 means the event is now empty and therefore can be deleted
#ifdef DEBUG
            log->dlog(ch,"remove event %s", eventNames[tmp->second->getType()].c_str());
#endif
           
            saveDelete(tmp->second);
            events.erase(tmp);
        } 
    }
}


Event *EventScheduler::getNextEvent()
{

    Event *ev;
    
    if (events.begin() != events.end()) {

        ev = events.begin()->second;

//#ifdef DEBUG
//        log->dlog(ch,"get Next Event %s", eventNames[ev->getType()].c_str());
//#endif	


        // dequeue event
        events.erase(events.begin());
        // the receiver is responsible for
        // returning or freeing the event
        return ev;
    } else {
        return NULL;
    }
}


void EventScheduler::reschedNextEvent(Event *ev)
{

#ifdef DEBUG
    log->dlog(ch,"starting requeue event %s", eventNames[ev->getType()].c_str());
#endif

/*
#ifdef DEBUG     
	eventListIter_t iter;
	
    // output all scheduled Events to ostream
    for (iter = events.begin(); iter != events.end(); iter++) {
        struct timeval rv = iter->first;
        log->dlog(ch,"Time:%s event:%s", (Timeval::toString(rv)).c_str(), 
					eventNames[iter->second->getType()].c_str());        
    }
#endif
*/
    assert(ev != NULL);
   
    if (ev->getIval() > 0) {

#ifdef DEBUG
        log->dlog(ch,"requeue event %s", eventNames[ev->getType()].c_str());
#endif	
        // recurring event so calculate next expiry time
        ev->advance();

#ifdef TIME_GAP_HACK
	struct timeval now;
	Timeval::gettimeofdayown(&now, NULL);
	struct timeval d = Timeval::sub0(now, ev->getTime());
	if (d.tv_sec > 0) {
	  ev->setTime(now);
	}
#endif

        // and requeue it
        events.insert(make_pair(ev->getTime(),ev));

/*
#ifdef DEBUG     
	log->dlog(ch,"after reschule the event");
	eventListIter_t iter;
	
    // output all scheduled Events to ostream
    for (iter = events.begin(); iter != events.end(); iter++) {
        struct timeval rv = iter->first;
        log->dlog(ch,"Time:%s event:%s", (Timeval::toString(rv)).c_str(), 
					eventNames[iter->second->getType()].c_str());        
    }
#endif
*/        
    } else {
#ifdef DEBUG
        log->dlog(ch,"remove event %s", eventNames[ev->getType()].c_str());
#endif
        saveDelete(ev);
    } 
}


void EventScheduler::delSessionEvents(int uid)
{
    int ret = 0;
    eventListIter_t iter, tmp;

    // search linearly through list for bid with given ID and delete entries
    iter = events.begin();
    while (iter != events.end()) {
        tmp = iter;
        iter++;
        
        ret = tmp->second->deleteSession(uid);
        if (ret == 1) {
            // ret = 1 means session was present in event but other sessions are still in
            // the event
#ifdef DEBUG
            log->dlog(ch,"remove session %d from event %s", uid, 
                      eventNames[tmp->second->getType()].c_str());
#endif
        } else if (ret == 2) {
            // ret=2 means the event is now empty and therefore can be deleted
#ifdef DEBUG
            log->dlog(ch,"remove event %s", eventNames[tmp->second->getType()].c_str());
#endif
           
            saveDelete(tmp->second);
            events.erase(tmp);
        } 
    }
} 


struct timeval EventScheduler::getNextEventTime()
{
    struct timeval rv = {0, MIN_TIMEOUT};
    struct timeval now;
    char c = 'A';

    if (events.begin() != events.end()) {
        Event *ev = events.begin()->second;
		Timeval::gettimeofdayown(&now, NULL);
		
        rv = Timeval::sub0(ev->getTime(), now);
        
        // be 100us fuzzy
        if ((rv.tv_sec == 0) && (rv.tv_usec<100)) {
#ifdef DEBUG
            log->dlog(ch,"expired event %s", eventNames[ev->getType()].c_str());
#endif
           //!* This lines activates the file descriptor being tracked by the 
           //!* the execution class. this methos should be redefined in every 
           //!* execution class, i.e., Auctioner, UserAgent.
           // write(Auctioner::s_sigpipe[1], &c, 1);
        }
        
    } 
    return rv;
}


/* ------------------------- dump ------------------------- */

void EventScheduler::dump(ostream &os)
{
    struct timeval now;
    eventListIter_t iter;
    
    gettimeofday(&now, NULL);
    
    os << "EventScheduler dump : \n";
    
    // output all scheduled Events to ostream
    for (iter = events.begin(); iter != events.end(); iter++) {
        struct timeval rv = Timeval::sub0(iter->first, now);
        os << "at t = " << rv.tv_sec * 1e6 + rv.tv_usec << " -> " 
           << eventNames[iter->second->getType()] << endl;
    }
}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< (ostream &os, EventScheduler &dc)
{
    dc.dump(os);
    return os;
}
