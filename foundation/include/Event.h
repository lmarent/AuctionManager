
/*! \file   Event.h

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
    event classes for all events handled by meter
    Code based on Netmate Implementation

    $Id: Event.h 748 2015-07-23 10:23:00 amarentes $
*/

#ifndef _EVENT_H_
#define _EVENT_H_


#include "stdincpp.h"
#include "ProcModuleInterface.h"
#include "AuctionFileParser.h"
#include "AuctionManagerInfo.h"
#include "Auction.h"
#include "BiddingObject.h"

namespace auction
{

//! event numbers
typedef enum 
{
      ADD_BIDDING_OBJECTS = 0,
      ADD_GENERATED_BIDDING_OBJECTS,
      REMOVE_BIDDING_OBJECTS,
      TRANSMIT_BIDDING_OBJECTS,
      ADD_BIDDING_OBJECT_AUCTION,
      REMOVE_BIDDING_OBJECT_AUCTION,
      ADD_AUCTIONS,
      REMOVE_AUCTIONS,
      ACTIVATE_AUCTIONS,
      ADD_RESOURCEREQUESTS,
      REMOVE_RESOURCEREQUESTS,
      ACTIVATE_RESOURCE_REQUEST_INTERVAL,
      REMOVE_RESOURCE_REQUEST_INTERVAL,
      PUSH_EXECUTION,
      GET_INFO,
      GET_MODINFO,
      TEST,
      REMOVE_BIDDING_OBJECTS_CTRLCOMM,
      ADD_BIDDING_OBJECTS_CTRLCOMM,
      REMOVE_AUCTIONS_CTRLCOMM,
      ADD_AUCTIONS_CNTRLCOMM,
      PROC_MODULE_TIMER,
      CTRLCOMM_TIMER,  
      CREATE_SESSION,
      CREATE_CHECK_SESSION,
      RESPONSE_CREATE_SESSION,
      RESPONSE_CREATE_CHECK_SESSION,
      REMOVE_SESSION,
      AUCTION_INTERACTION    
} event_t;

//! event names for dump method
const string eventNames[] = 
{
      "Add-Bidding-Objects",
      "Add-Generated-Bidding-Objects",
      "Remove-Bidding-Objects",
      "Transmit-Bidding-Objects",
      "Add-Bidding-Object-Auction",
      "Delete-Bidding-Object-Auction",
      "Add-Auctions",
      "Remove-Auctions",
      "Activate-Auctions",
      "Add-ResourceRequests",
      "Remove-ResourceRequests",
      "Activate-ResourceRequests-Interval",
      "Delete-ResourceRequest-Interval",
      "Push-Execution",
      "Get-info",
      "Get-module-info",
      "Test",
      "Remove-Bidding-Objects-ctrlcomm",
      "Add-Bidding-Objects-ctrlcomm",
      "Remove-Auctions-ctrlcomm",
      "Add-Auctions-ctrlcomm",
      "Proc-module-timer",
      "Ctrlcomm-timer",
      "Create-Session",
      "Create-Check-Session",
      "Response-Create-Session",
      "Response-Create-Check-Session",
      "Remove-Session",
      "Auction-Interaction",
};

/* ------------------------- Event class ------------------------- */

/*! \short   basic event element that is the base class of all 
             other events that can be stored in the event queue 
             of EventScheduler
*/

class Event {
      
  private:

    //! type of the event   
    event_t type;    
    
    //!< time for next processing of event [sec since Epoch]
    struct timeval when;
    
    //!< interval between two event processings [msec]
    unsigned long interval;
    
    //! align events on time boundaries
    void doAlign();
    
  public:
  
    /*! \short  create an event at an absolute time
        \arg \c type  type of the event
        \arg \c time  absolute timestamp when the event is due
        \arg \c ival  interval (in ms) if the event is recurrent
        \arg \c align  align the event on the next ival
    */
    Event(event_t type, struct timeval time, unsigned long ival=0, 
	  int align=0);
    
    /*! \short  create an event relative to current time
        \arg \c type  type of the event
        \arg \c offs_sec  sec offset from now when the event is due
        \arg \c offs_used  usec offset from now when the event is due
        \arg \c ival  interval (in ms) if the event is recurrent
        \arg \c align  align the event on the next ival
    */
    Event(event_t type, time_t offs_sec, time_t offs_usec = 0, 
    	  unsigned long ival=0, int align=0);
    
    /*! \short  create an event at the current time (now)
        \arg \c type  type of the event
        \arg \c ival  interval (in ms) if the event is recurrent
        \arg \c align  align the event on the next ival
    */
    Event(event_t type, unsigned long ival=0, int align=0);
    
    virtual ~Event() {}
    
    //! get event type
    event_t getType() 
    { 
        return type;
    }

    int isType( event_t atype )
    { 
        return (type == atype);
    }

    //! get expiry time
    struct timeval getTime()                        
    {
        return when;
    }
    
    //! get interval
    unsigned long getIval()                        
    {
        return interval;
    }
    
    //! set interval
    void setInterval( unsigned long ival )        
    {
        interval = ival;
    }
    
    //! set expiry time
    void setTime( struct timeval newTime )         
    {
        when = newTime;
    }
    
    //! set expiry time (full sec)
    void setTime(time_t newTimeSec)
    { 
        when.tv_sec = newTimeSec;
        when.tv_usec = 0;
    }
    
    //! get next expiry time (recurrent events)
    void advance();
    
    virtual void dump( ostream &os );
    
    //! delete biddingObjects stored in this event
    virtual int deleteBiddingObject(int uid) 
    {
	return 0;
    }

    //! delete auctions stored in this event
    virtual int deleteAuction(int uid) 
    {
	return 0;
    }
    
    //! delete resource request stored in this event
    virtual int deleteResourceRequest(int uid)
    {
	return 0;
	}
	
	//! delete a session stored in this event
	virtual int deleteSession(int uid)
	{
	return 0;
	}
};


//! base class for all ctrlcomm events, contains pointer to request
class CtrlCommEvent: public Event
{
  private:  
    struct REQUEST *req;

  public:
    //! ctrlcomm events always expire now
    CtrlCommEvent(event_t type, unsigned long ival=0, int align=0)
      : Event(type, ival, align) {}

    virtual ~CtrlCommEvent() {}

    //! get request pointer
    struct REQUEST *getReq()
    { 
	return req;
    }
    
    //! set request pointer
    void setReq(struct REQUEST *r)
    {
	req = r;
    }
};

//! test event
class TestEvent : public Event
{
  private:
   
  public:

    TestEvent(struct timeval time, unsigned long ival=0)
      : Event(TEST, time, ival) {}

    TestEvent(time_t offs_sec, time_t offs_usec, unsigned long ival=0)
      : Event(TEST, offs_sec, offs_usec, ival) {}

    TestEvent(unsigned long ival=0)
      : Event(TEST, ival) {}
};


/* --------------------------------- events ------------------------------ */


class AddBiddingObjectsEvent : public Event
{
  private:
    string fileName;

  public:

    AddBiddingObjectsEvent(string fname, int mapi=0) 
      : Event(ADD_BIDDING_OBJECTS), fileName(fname) 
    {
        
    }

    string getFileName()
    {
        return fileName;
    }
};

class AddGeneratedBiddingObjectsEvent : public Event
{
  private:
    biddingObjectDB_t biddingObjects;

  public:

    AddGeneratedBiddingObjectsEvent( biddingObjectDB_t &biddingObjects ): 
    Event(ADD_GENERATED_BIDDING_OBJECTS), biddingObjects(biddingObjects) 
    {
        
    }

    biddingObjectDB_t *getBiddingObjects()
    {
        return &biddingObjects;
    }
    
    int deleteBiddingObject(int uid)
    {
        int ret = 0;
        biddingObjectDBIter_t iter;
        
        for (iter=biddingObjects.begin(); iter != biddingObjects.end(); iter++) {
            if ((*iter)->getUId() == uid) {
                biddingObjects.erase(iter);
                ret++;
                break;
            }   
        }
          
        if (biddingObjects.empty()) {
            return ++ret;
        }
          
        return ret;
    }
};


class RemoveBidsEvent : public Event
{
  private:
    biddingObjectDB_t biddingObjects;

  public:

    RemoveBidsEvent(struct timeval time, biddingObjectDB_t &b) 
      : Event(REMOVE_BIDDING_OBJECTS, time), biddingObjects(b) {}

    RemoveBidsEvent(time_t offs_sec, biddingObjectDB_t &b) 
      : Event(REMOVE_BIDDING_OBJECTS, offs_sec), biddingObjects(b) {}
    
    RemoveBidsEvent(biddingObjectDB_t &b) 
      : Event(REMOVE_BIDDING_OBJECTS), biddingObjects(b) {}

    biddingObjectDB_t *getBiddingObjects()
    {
        return &biddingObjects;
    }
    
    int deleteBiddingObject(int uid)
    {
        int ret = 0;
        biddingObjectDBIter_t iter;
        
        for (iter=biddingObjects.begin(); iter != biddingObjects.end(); iter++) {
            if ((*iter)->getUId() == uid) {
                biddingObjects.erase(iter);
                ret++;
                break;
            }   
        }
          
        if (biddingObjects.empty()) {
            return ++ret;
        }
          
        return ret;
    }
};

class TransmitBiddingObjectsEvent : public Event
{
  private:
    biddingObjectDB_t biddingObjects;

  public:
    
    TransmitBiddingObjectsEvent(biddingObjectDB_t &b) 
      : Event(TRANSMIT_BIDDING_OBJECTS), biddingObjects(b) {}

    biddingObjectDB_t *getBiddingObjects()
    {
        return &biddingObjects;
    }
    
    int deleteBiddingObject(int uid)
    {
        int ret = 0;
        biddingObjectDBIter_t iter;
        
        for (iter=biddingObjects.begin(); iter != biddingObjects.end(); iter++) {
            if ((*iter)->getUId() == uid) {
                biddingObjects.erase(iter);
                ret++;
                break;
            }   
        }
          
        if (biddingObjects.empty()) {
            return ++ret;
        }
          
        return ret;
    }
};


class AddAuctionsEvent : public Event
{
  private:
    string fileName;

  public:

    AddAuctionsEvent(string fname, int mapi=0) 
      : Event(ADD_AUCTIONS), fileName(fname) 
    {
        
    }

    string getFileName()
    {
        return fileName;
    }
};


class ActivateAuctionsEvent : public Event
{
  private:
    auctionDB_t auctions;

  public:

    ActivateAuctionsEvent(struct timeval time, auctionDB_t &a) 
      : Event(ACTIVATE_AUCTIONS, time), auctions(a) {}

     ActivateAuctionsEvent(time_t offs_sec, auctionDB_t &a) 
      : Event(ACTIVATE_AUCTIONS, offs_sec), auctions(a) {}

     ActivateAuctionsEvent(auctionDB_t &a) 
      : Event(ACTIVATE_AUCTIONS), auctions(a) {}

     auctionDB_t *getAuctions()
     {
         return &auctions;
     }

     int deleteAuction(int uid)
     {
         int ret = 0;
         auctionDBIter_t iter;
           
         for (iter=auctions.begin(); iter != auctions.end(); iter++) {
             if ((*iter)->getUId() == uid) {
                 auctions.erase(iter);
                 ret++;
                 break;
             }   
         }
         
         if (auctions.empty()) {
             return ++ret;
         }
         
         return ret;
     }
};


class RemoveAuctionsEvent : public Event
{
  private:
    auctionDB_t auctions;

  public:

    RemoveAuctionsEvent(struct timeval time, auctionDB_t &a) 
      : Event(REMOVE_AUCTIONS, time), auctions(a) {}

    RemoveAuctionsEvent(time_t offs_sec, auctionDB_t &a) 
      : Event(REMOVE_AUCTIONS, offs_sec), auctions(a) {}
    
    RemoveAuctionsEvent(auctionDB_t &a) 
      : Event(REMOVE_AUCTIONS), auctions(a) {}

    auctionDB_t *getAuctions()
    {
        return &auctions;
    }
    
    int deleteAuction(int uid)
    {
        int ret = 0;
        auctionDBIter_t iter;
        
        for (iter=auctions.begin(); iter != auctions.end(); iter++) {
            if ((*iter)->getUId() == uid) {
                auctions.erase(iter);
                ret++;
                break;
            }   
        }
          
        if (auctions.empty()) {
            return ++ret;
        }
          
        return ret;
    }
};

class InsertBiddingObjectAuctionEvent : public Event
{
  private:
	BiddingObject * biddingObject;
	string auctionSet;
	string auctionName;
	
  public:

    InsertBiddingObjectAuctionEvent(struct timeval time, 
									BiddingObject *_biddingObject, 
									string _auctionSet, 
									string _auctionName) : 
		Event(ADD_BIDDING_OBJECT_AUCTION, time), biddingObject(_biddingObject), auctionSet(_auctionSet),
		auctionName(_auctionName){}

     InsertBiddingObjectAuctionEvent(time_t offs_sec, 
									 BiddingObject *_biddingObject, 
									 string _auctionSet, 
									 string _auctionName) : 
		Event(ADD_BIDDING_OBJECT_AUCTION, offs_sec), biddingObject(_biddingObject), auctionSet(_auctionSet),
		auctionName(_auctionName) {}

     InsertBiddingObjectAuctionEvent(BiddingObject *_biddingObject, 
									 string _auctionSet, 
									 string _auctionName ) : 
		Event(ADD_BIDDING_OBJECT_AUCTION), biddingObject(_biddingObject), auctionSet(_auctionSet),
		auctionName(_auctionName) {}

	 BiddingObject * getBiddingObject()
	 {
	     return biddingObject;
	 }
	 
	 string getAuctionSet()
	 {
		 return auctionSet;
	 }
	 
	 string getAuctionName()
	 {
		 return auctionName;
	 }

};


class RemoveBiddingObjectAuctionEvent : public Event
{
  private:
	BiddingObject * biddingObject;
	string auctionSet;
	string auctionName;
	
  public:

     RemoveBiddingObjectAuctionEvent(struct timeval time, 
									 BiddingObject *_biddingObject, 
									 string _auctionSet, 
									 string _auctionName) : 
		Event(REMOVE_BIDDING_OBJECT_AUCTION, time), biddingObject(_biddingObject), 
			auctionSet(_auctionSet), auctionName(_auctionName){}

     RemoveBiddingObjectAuctionEvent(time_t offs_sec, 
									 BiddingObject *_biddingObject, 
									 string _auctionSet, 
									 string _auctionName) : 
		Event(REMOVE_BIDDING_OBJECT_AUCTION, offs_sec), biddingObject(_biddingObject),
			auctionSet(_auctionSet), auctionName(_auctionName) {}

     RemoveBiddingObjectAuctionEvent(BiddingObject *_biddingObject, 
									 string _auctionSet, 
									 string _auctionName ) :
		Event(REMOVE_BIDDING_OBJECT_AUCTION), biddingObject(_biddingObject), 
			auctionSet(_auctionSet), auctionName(_auctionName) {}

	 BiddingObject * getBiddingObject()
	 {
		 return biddingObject;
	 }
	 
	 string getAuctionSet()
	 {
		 return auctionSet;
	 }
	 
	 string getAuctionName()
	 {
		 return auctionName;
	 }
};


class CtrlCommTimerEvent : public Event
{
    
  public:
    
    CtrlCommTimerEvent(time_t offs_sec, unsigned long ival=0, int align=0) 
      : Event(CTRLCOMM_TIMER, offs_sec,0,ival,align) {}
};



//! add biddingObjects flags
const int ADD_BIDS_MAPI   = 0x1;

class AddBiddingObjectsCtrlEvent : public CtrlCommEvent
{
  private:
    int type;
    char *buf;
    int len;

  public:

    AddBiddingObjectsCtrlEvent(char *b, int l)
      : CtrlCommEvent(ADD_BIDDING_OBJECTS_CTRLCOMM), type(0), len(l) 
    {
        buf = new char[len+1];
        memcpy(buf, b, len+1);
          
    }

    ~AddBiddingObjectsCtrlEvent()
    {
        saveDeleteArr(buf);
    }

    char *getBuf()
    {
        return buf;
    }

    int getLen()
    {
        return len;
    }
};


class RemoveBiddingObjectsCtrlEvent : public CtrlCommEvent
{
  private:
    string biddingObject;

  public:
    
    RemoveBiddingObjectsCtrlEvent(string b) : 
		CtrlCommEvent(REMOVE_BIDDING_OBJECTS_CTRLCOMM), biddingObject(b) {}

    string getBiddingObject()
    {
        return biddingObject;
    }
};


//! add auction flags
const int ADD_AUCTIONS_MAPI   = 0x2;

class AddAuctionsCtrlEvent : public CtrlCommEvent
{
  private:
    int type;
    char *buf;
    int len;

  public:

    AddAuctionsCtrlEvent(char *b, int l, int mapi=0)
      : CtrlCommEvent(ADD_AUCTIONS_CNTRLCOMM), type(0), len(l) 
    {
        buf = new char[len+1];
        memcpy(buf, b, len+1);
          
        if (mapi) {
            type |= ADD_AUCTIONS_MAPI;
        }
    }

    ~AddAuctionsCtrlEvent()
    {
        saveDeleteArr(buf);
    }

    int isMAPI()
    {
        return (type & ADD_AUCTIONS_MAPI);
    }

    char *getBuf()
    {
        return buf;
    }

    int getLen()
    {
        return len;
    }
};


class RemoveAuctionsCtrlEvent : public CtrlCommEvent
{
  private:
    string auction;

  public:
    
    RemoveAuctionsCtrlEvent(string a) 
      : CtrlCommEvent(REMOVE_AUCTIONS_CTRLCOMM), auction(a) {}

    string getAuction()
    {
        return auction;
    }
};


//! overload for << so that an Event object can be thrown into an ostream
ostream& operator<< ( ostream &os, Event &ev );

}; // namespace auction.

#endif // _EVENT_H_
