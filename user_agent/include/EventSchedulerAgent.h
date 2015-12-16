
/*!\file   EventSchedulerAgent.h

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

    $Id: EventSchedulerAgent.h 748 2015-08-25 08:34:00 amarentes $
*/

#ifndef _EVENT_SCHEDULER_AGENT_H_
#define _EVENT_SCHEDULER_AGENT_H_

#include "EventScheduler.h"

namespace auction
{

class EventSchedulerAgent : public EventScheduler
{

public:

	EventSchedulerAgent();
	
    //! return the time of the next event due
    struct timeval getNextEventTime();
	
    /*! \short   delete all events for a given resource request

        delete all Events related to the specified resource request from the list of events

        \arg \c uid  - the unique identification number of the resource request
    */
    void delResourceRequestEvents(int uid);
	
	
	void rescheduleAuctionDelete(int uid, time_t stop);
};

}; // namespace auction

#endif // _EVENT_SCHEDULER_AGENT_H_
