
/*! \file   EventAgent.h

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Quality Manager System (NETAUM).

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
    EventAgent classes for all specific events handled by an agent.
    Code based on Netmate Implementation

    $Id: EventAgent.h 748 2015-08-25 13:40:00 amarentes $
*/

#ifndef _EVENT_AGENT_H_
#define _EVENT_AGENT_H_


#include "stdincpp.h"
#include "ProcModuleInterface.h"
#include "ResourceRequestFileParser.h"
#include "AuctionFileParser.h"
#include "AuctionManagerInfo.h"
#include "Auction.h"
#include "Event.h"

namespace auction
{

/* --------------------------------- events ------------------------------ */


class GetInfoEvent : public CtrlCommEvent
{
  private:
    agentInfoList_t *infos;
   
  public:

    GetInfoEvent(agentInfoList_t *i)
      : CtrlCommEvent(GET_INFO), infos(i) {}

    ~GetInfoEvent()
    {
        saveDelete(infos);
    }

    agentInfoList_t *getInfos()
    {
        return infos;
    }
};


class ProcTimerEvent : public Event
{
private:	
    unsigned int tmID;
    timeout_func_t tmFunc;
    
public:

    ProcTimerEvent( timeout_func_t timeout, timers_t *timer ) :
      Event( PROC_MODULE_TIMER,
             timer->ival_msec/1000, timer->ival_msec%1000,
             ((timer->flags & TM_RECURRING) ? timer->ival_msec : 0),
             timer->flags & TM_ALIGNED),
      tmID(timer->id),
      tmFunc(timeout)
      {}
    
    int signalTimeout() { return tmFunc( tmID ); }

};

class AddResourceRequestsEvent : public Event
{
  private:
    string fileName;

  public:

    AddResourceRequestsEvent(string fname, int mapi=0) 
      : Event(ADD_RESOURCEREQUESTS), fileName(fname) 
    {
        
    }

    string getFileName()
    {
        return fileName;
    }
};


class RemoveResourceRequestsEvent : public Event
{
  private:
    resourceRequestDB_t requests;

  public:

    RemoveResourceRequestsEvent(struct timeval time, resourceRequestDB_t &r) 
      : Event(REMOVE_RESOURCEREQUESTS, time), requests(r) {}

    RemoveResourceRequestsEvent(time_t offs_sec, resourceRequestDB_t &r) 
      : Event(REMOVE_RESOURCEREQUESTS, offs_sec), requests(r) {}
    
    RemoveResourceRequestsEvent(resourceRequestDB_t &r) 
      : Event(REMOVE_RESOURCEREQUESTS), requests(r) {}

    resourceRequestDB_t *getResourceRequests()
    {
        return &requests;
    }
    
    int deleteResourceRequest(int uid)
    {
        int ret = 0;
        resourceRequestDBIter_t iter;
        
        for (iter=requests.begin(); iter != requests.end(); iter++) {
            if ((*iter)->getUId() == uid) {
                requests.erase(iter);
                ret++;
                break;
            }   
        }
          
        if (requests.empty()) {
            return ++ret;
        }
          
        return ret;
    }
};

}; // namespace auction

#endif // _EVENT_AGENT_H_
