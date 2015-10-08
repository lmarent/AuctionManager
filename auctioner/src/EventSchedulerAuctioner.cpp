
/*!\file   EventSchedulerAuctioner.cpp

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

    $Id: EventSchedulerAuctioner.cpp 748 2015-08-24 19:13:00 amarentes $
*/


#include "ParserFcts.h"
#include "EventSchedulerAuctioner.h"
#include "Auctioner.h"

using namespace auction;

// min timeout for select() in us (10ms minimum on current UNIX!)
const int AUCTIONER_MIN_TIMEOUT = 10000;

EventSchedulerAuctioner::EventSchedulerAuctioner(): EventScheduler()
{

}

struct timeval EventSchedulerAuctioner::getNextEventTime()
{
    struct timeval rv = {0, AUCTIONER_MIN_TIMEOUT};
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
           write(Auctioner::s_sigpipe[1], &c, 1);
        }
        
    } 
    return rv;
}

