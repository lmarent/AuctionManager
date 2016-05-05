
/*!\file   EventSchedulerAuctioner.h

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

    $Id: EventSchedulerAuctioner.h 748 2015-08-24 19:13:00 amarentes $
*/

#ifndef _EVENTSCHEDULER_AUCTIONER_H_
#define _EVENTSCHEDULER_AUCTIONER_H_

#include "EventScheduler.h"

namespace auction
{

class EventSchedulerAuctioner : public EventScheduler
{

public:

	EventSchedulerAuctioner();
	
    //! return the time of the next event due
    struct timeval getNextEventTime();


    /*! \short   delete all events for a given process index

        delete all Events related to the specified process index from the list of events

        \arg \c uid  - the unique identification number of the process index
    */
    void delProcessExecutionEvents(int uid);
		
};

}; // namespace auction

#endif // _EVENTSCHEDULER_AUCTIONER_H_
