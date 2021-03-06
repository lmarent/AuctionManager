
/*! \file   EventAuctioner.h

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
    EventAuctioner classes for all specific events handled by auctioner.
    Code based on Netmate Implementation

    $Id: EventAuctioner.h 748 2015-07-23 18:14:00 amarentes $
*/

#ifndef _EVENT_AUCTIONER_H_
#define _EVENT_AUCTIONER_H_


#include "ParserFcts.h"
#include "stdincpp.h"
#include "ProcModule.h"
#include "ProcModuleInterface.h"
#include "BiddingObjectFileParser.h"
#include "AuctionFileParser.h"
#include "AuctionManagerInfo.h"
#include "Auction.h"
#include "Event.h"
#include "aqueue.h"

namespace auction
{

/* --------------------------------- events ------------------------------ */

class CreateSessionEvent : public Event
{
  private:
	string sessionId;
	anslp::objectList_t objects;
	anslp::FastQueue *ret;
    
  public:

    CreateSessionEvent(string _sessionId, anslp::FastQueue *_ret, unsigned long ival=0, int align=0) 
      : Event(CREATE_SESSION, ival, align), sessionId(_sessionId), ret(_ret) {  }

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



class CreateCheckSessionEvent : public Event
{
  private:
	string sessionId;
	anslp::objectList_t objects;
	anslp::FastQueue *ret;
    
  public:

    CreateCheckSessionEvent(string _sessionId, anslp::FastQueue *_ret, unsigned long ival=0, int align=0) 
      : Event(CREATE_CHECK_SESSION, ival, align), sessionId(_sessionId), ret(_ret){  }
  
    ~CreateCheckSessionEvent() 
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


class PushExecutionEvent : public Event
{
  private:
	int index;
	time_t stop;
	
  public:

    PushExecutionEvent(struct timeval time, int _index, time_t _stop, unsigned long ival=0, int align=0) 
      : Event(PUSH_EXECUTION, time, ival, align), index(_index), stop() {  }

    PushExecutionEvent(time_t offs_sec, int _index, time_t _stop, unsigned long ival=0, int align=0) 
      : Event(PUSH_EXECUTION, offs_sec, 0, ival, align), index(_index), stop(_stop) { }

    PushExecutionEvent(int _index, time_t _stop, unsigned long ival=0, int align=0) 
      : Event(PUSH_EXECUTION, ival, align), index(_index), stop(_stop) { }

    int getIndex()
    {
        return index;
    }
    
    time_t getStop(){
		return stop;
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


/* ------------------------------- ctrlcomm events ------------------------ */

class GetInfoEvent : public CtrlCommEvent
{
  private:
    infoList_t *infos;
   
  public:

    GetInfoEvent(infoList_t *i)
      : CtrlCommEvent(GET_INFO), infos(i) {}

    ~GetInfoEvent()
    {
        saveDelete(infos);
    }

    infoList_t *getInfos()
    {
        return infos;
    }
};


class GetModInfoEvent : public CtrlCommEvent
{
  private:

    string modname;
    
  public:

    GetModInfoEvent( string modulename )
      : CtrlCommEvent(GET_MODINFO), modname(modulename) {}

    ~GetModInfoEvent() {}

    string getModName()
    {
        return modname;
    }
};

class AddResourceEvent : public Event
{
  private:
    string fileName;

  public:

    AddResourceEvent(string fname, int mapi=0) 
      : Event(ADD_RESOURCE), fileName(fname) 
    {
        
    }

    string getFileName()
    {
        return fileName;
    }
};

class AddResourceCtrlCommEvent : public CtrlCommEvent
{
  private:
    string xml;

  public:

    AddResourceCtrlCommEvent(string xml) 
      : CtrlCommEvent(ADD_RESOURCE_CTRLCOMM), xml(xml) 
    {
        
    }

    string getMessage()
    {
        return xml;
    }
};


class ActivateResourceEvent : public Event
{
  private:
    auctioningObjectDB_t resources;
    time_t startTime;

  public:

    ActivateResourceEvent(struct timeval time, auctioningObjectDB_t resources, time_t _startTime) 
      : Event(ACTIVATE_RESOURCE, time), resources(resources), startTime(_startTime) {  }

    ActivateResourceEvent(time_t offs_sec, time_t offs_usec, auctioningObjectDB_t resources, time_t _startTime) 
      : Event(ACTIVATE_RESOURCE, offs_sec, offs_usec), resources(resources), startTime(_startTime) {}

    ActivateResourceEvent(auctioningObjectDB_t resources,  time_t _startTime) 
      : Event(ACTIVATE_RESOURCE), resources(resources), startTime(_startTime) {}

     auctioningObjectDB_t *getResource()
     {
         return &resources;
     }
     
     time_t getStartTime()
     {
		return startTime;
	 }
     
    int deleteResource(int uid)
    {
        int ret = 0;
        auctioningObjectDBIter_t iter;
        
        for (iter=resources.begin(); iter != resources.end(); iter++) {
            if ((*iter)->getUId() == uid) {
                resources.erase(iter);
                ret++;
                break;
            }   
        }
          
        if (resources.empty()) {
            return ++ret;
        }
          
        return ret;
    }

};


class RemoveResourceEvent : public Event
{
  private:
    auctioningObjectDB_t resources;

  public:

    RemoveResourceEvent(struct timeval time, auctioningObjectDB_t &r) 
      : Event(REMOVE_RESOURCE, time), resources(r) {}

    RemoveResourceEvent(time_t offs_sec, auctioningObjectDB_t &r) 
      : Event(REMOVE_RESOURCE, offs_sec), resources(r) {}
    
    RemoveResourceEvent(auctioningObjectDB_t &r) 
      : Event(REMOVE_RESOURCE), resources(r) {}

    auctioningObjectDB_t *getResources()
    {
        return &resources;
    }
    
    int deleteResource(int uid)
    {
        int ret = 0;
        auctioningObjectDBIter_t iter;
        
        for (iter=resources.begin(); iter != resources.end(); iter++) {
            if ((*iter)->getUId() == uid) {
                resources.erase(iter);
                ret++;
                break;
            }   
        }
          
        if (resources.empty()) {
            return ++ret;
        }
          
        return ret;
    }
};



}; // namespace auction

#endif // _EVENT_AUCTIONER_H_
