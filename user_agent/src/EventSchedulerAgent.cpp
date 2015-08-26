
/*!\file   EventSchedulerAgent.cpp

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

    $Id: EventSchedulerAgent.cpp 748 2015-08-25 08:32:00 amarentes $
*/

#include "EventSchedulerAgent.h"
#include "Agent.h"

// min timeout for select() in us (10ms minimum on current UNIX!)
const int AGENT_MIN_TIMEOUT = 10000;

EventSchedulerAgent::EventSchedulerAgent(): EventScheduler()
{

}

struct timeval EventSchedulerAgent::getNextEventTime()
{
    struct timeval rv = {0, AGENT_MIN_TIMEOUT};
    struct timeval now;
    char c = 'A';

    if (events.begin() != events.end()) {
        Event *ev = events.begin()->second;
		Timeval::gettimeofday(&now, NULL);
		
        rv = Timeval::sub0(ev->getTime(), now);
        
        // be 100us fuzzy
        if ((rv.tv_sec == 0) && (rv.tv_usec<100)) {
#ifdef DEBUG
            log->dlog(ch,"expired event %s", eventNames[ev->getType()].c_str());
#endif
           write(Agent::s_sigpipe[1], &c, 1);
        }
        
    } 
    return rv;
}

void EventSchedulerAgent::delResourceRequestEvents(int uid)
{
    int ret = 0;
    eventListIter_t iter, tmp;

    // search linearly through list for resource request with given ID and delete entries
    iter = events.begin();
    while (iter != events.end()) {
        tmp = iter;
        iter++;
        
        ret = tmp->second->deleteResourceRequest(uid);
        if (ret == 1) {
            // ret = 1 means rule was present in event but other rules are still in
            // the event
#ifdef DEBUG
            log->dlog(ch,"remove resource request %d from event %s", uid, 
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
 
