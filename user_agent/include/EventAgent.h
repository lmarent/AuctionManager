
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
#include "AgentManagerInfo.h"
#include "Auction.h"
#include "Event.h"
#include "aqueue.h"

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

class AddResourceRequestsCtrlCommEvent : public CtrlCommEvent
{
  private:
    string xml;

  public:

    AddResourceRequestsCtrlCommEvent(string xml) 
      : CtrlCommEvent(ADD_RESOURCEREQUESTS_CTRLCOMM), xml(xml) 
    {
        
    }

    string getMessage()
    {
        return xml;
    }
};


class ActivateResourceRequestIntervalEvent : public Event
{
  private:
    ResourceRequest *request;
    time_t startTime;

  public:

    ActivateResourceRequestIntervalEvent(struct timeval time, ResourceRequest *request, time_t _startTime) 
      : Event(ACTIVATE_RESOURCE_REQUEST_INTERVAL, time), request(request), startTime(_startTime) {  }

    ActivateResourceRequestIntervalEvent(time_t offs_sec, time_t offs_usec, ResourceRequest *request, time_t _startTime) 
      : Event(ACTIVATE_RESOURCE_REQUEST_INTERVAL, offs_sec, offs_usec), request(request), startTime(_startTime) {}

    ActivateResourceRequestIntervalEvent(ResourceRequest *request,  time_t _startTime) 
      : Event(ACTIVATE_RESOURCE_REQUEST_INTERVAL), request(request), startTime(_startTime) {}

     ResourceRequest *getResourceRequest()
     {
         return request;
     }
     
     time_t getStartTime()
     {
		return startTime;
	 }
     
     int deleteResourceRequest(int uid)
     {
         int ret = 0;
         
         if (request != NULL){
			if ((request)->getUId() == uid){
				request = NULL;
			}
         }
         return ret;
     }
};


class RemoveResourceRequestIntervalEvent : public Event
{
  private:
    ResourceRequest *request;
    time_t stopTime;

  public:

    RemoveResourceRequestIntervalEvent(struct timeval time, ResourceRequest *r, time_t _stoptime) 
      : Event(REMOVE_RESOURCEREQUESTS, time), request(r), stopTime(_stoptime) {}

    RemoveResourceRequestIntervalEvent(time_t offs_sec, time_t offs_usec,  ResourceRequest *r, time_t _stoptime) 
      : Event(REMOVE_RESOURCE_REQUEST_INTERVAL, offs_sec, offs_usec), request(r), stopTime(_stoptime) {}

    RemoveResourceRequestIntervalEvent(ResourceRequest *r, time_t _stoptime) 
      : Event(REMOVE_RESOURCEREQUESTS), request(r), stopTime(_stoptime) {}

     ResourceRequest *getResourceRequest()
     {
         return request;
     }

     time_t getStopTime()
     {
		return stopTime;
	 }

     int deleteResourceRequest(int uid)
     {
         int ret = 0;
         
         if (request != NULL){
			if ((request)->getUId() == uid){
				request = NULL;
			}
         }
         return ret;
     }
};



class RemoveResourceRequestsEvent : public Event
{
  private:
    auctioningObjectDB_t requests;

  public:

    RemoveResourceRequestsEvent(struct timeval time, auctioningObjectDB_t &r) 
      : Event(REMOVE_RESOURCEREQUESTS, time), requests(r) {}

    RemoveResourceRequestsEvent(time_t offs_sec, auctioningObjectDB_t &r) 
      : Event(REMOVE_RESOURCEREQUESTS, offs_sec), requests(r) {}
    
    RemoveResourceRequestsEvent(auctioningObjectDB_t &r) 
      : Event(REMOVE_RESOURCEREQUESTS), requests(r) {}

    auctioningObjectDB_t *getResourceRequests()
    {
        return &requests;
    }
    
    int deleteResourceRequest(int uid)
    {
        int ret = 0;
        auctioningObjectDBIter_t iter;
        
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

class CreateSessionEvent : public Event
{
  private:
    
    string sessionId; 
    anslp::objectList_t objects;
    anslp::FastQueue *ret;

  public:

    CreateSessionEvent(string _sessionId, anslp::FastQueue *_ret, unsigned long ival=0, int align=0) 
      : Event(CREATE_SESSION, ival, align), sessionId(_sessionId), ret(_ret) 
    {
        
    }

    ~CreateSessionEvent() 
    {
		anslp::objectListIter_t it;
		for ( it = objects.begin(); it != objects.end(); it++)
		{
			if (it->second != NULL)
				delete(it->second);
		}
	}

	void setObject(anslp::mspec_rule_key key, anslp::msg::anslp_mspec_object *obj)
	{
		if ( obj == NULL )
		return;

		anslp::msg::anslp_mspec_object *old = objects[key];

		if ( old )
			delete old;

		objects[key] = obj;

	}

    anslp::objectList_t *  getObjects()
    {
        return &objects;
    }
    
    string getSessionId()
    {
		return sessionId;
	}

	anslp::FastQueue * getQueue()
	{
		return ret;
	}
	
};

class ConfigureSessionEvent : public Event
{
  private:
    
    string sessionId; 
    string anslpSessionId;

  public:

    ConfigureSessionEvent(string _sessionId, string _sid, unsigned long ival=0, int align=0) 
      : Event(CONFIGURE_SESSION, ival, align), sessionId(_sessionId), anslpSessionId(_sid) 
    {
        
    }

    ~ConfigureSessionEvent() 
    {

	}
    
    string getSessionId()
    {
		return sessionId;
	}

	string getAnslpSession()
	{
		return anslpSessionId;
	}
	
};



class PushExecutionEvent : public Event
{
  private:
    int index;

  public:

    PushExecutionEvent(struct timeval time, int _index, unsigned long ival=0, int align=0) 
      : Event(PUSH_EXECUTION, time, ival, align), index(_index) {  }

    PushExecutionEvent(time_t offs_sec, int _index, unsigned long ival=0, int align=0) 
      : Event(PUSH_EXECUTION, offs_sec, 0, ival, align), index(_index) { }

    PushExecutionEvent(int _index, unsigned long ival=0, int align=0) 
      : Event(PUSH_EXECUTION, ival, align), index(_index) { }

    int getIndex()
    {
        return index;
    }
};


class RemovePushExecutionEvent : public Event
{
  private:
    int index;

  public:

    RemovePushExecutionEvent(struct timeval time, int _index, unsigned long ival=0, int align=0) 
      : Event(REMOVE_PUSH_EXECUTION, time, ival, align), index(_index) {  }

    RemovePushExecutionEvent(time_t offs_sec, int _index, unsigned long ival=0, int align=0) 
      : Event(REMOVE_PUSH_EXECUTION, offs_sec, 0, ival, align), index(_index) { }

    RemovePushExecutionEvent(int _index, unsigned long ival=0, int align=0) 
      : Event(REMOVE_PUSH_EXECUTION, ival, align), index(_index) { }

    int getIndex()
    {
        return index;
    }
};


}; // namespace auction

#endif // _EVENT_AGENT_H_
