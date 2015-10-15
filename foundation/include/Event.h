
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
#include "BidFileParser.h"
#include "AuctionFileParser.h"
#include "AuctionManagerInfo.h"
#include "Auction.h"

namespace auction
{

//! event numbers
typedef enum 
{
      ADD_BIDS = 0,
      REMOVE_BIDS,
      ADD_BID_AUCTION,
      REMOVE_BID_AUCTION,
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
      REMOVE_BIDS_CTRLCOMM,
      ADD_BIDS_CTRLCOMM,
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
      "Add-bids",
      "Remove-bids",
      "Add-Bid-Auction",
      "Delete-Bid-Auction",
      "Add-Auctions",
      "Remove-Auctions",
      "Activate-Auctions",
      "Add-ResourceRequests",
      "Remove-ResourceRequests",
      "Activate-ResourceRequests",
      "Push-Execution",
      "Get-info",
      "Get-module-info",
      "Test",
      "Remove-bids-ctrlcomm",
      "Add-bids-ctrlcomm",
      "Remove-Auctions-ctrlcomm",
      "Add-Auctions-ctrlcomm",
      "Proc-module-timer",
      "Ctrlcomm-timer",      
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
    
    //! delete bids stored in this event
    virtual int deleteBid(int uid) 
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


class AddBidsEvent : public Event
{
  private:
    string fileName;

  public:

    AddBidsEvent(string fname, int mapi=0) 
      : Event(ADD_BIDS), fileName(fname) 
    {
        
    }

    string getFileName()
    {
        return fileName;
    }
};


class RemoveBidsEvent : public Event
{
  private:
    bidDB_t bids;

  public:

    RemoveBidsEvent(struct timeval time, bidDB_t &b) 
      : Event(REMOVE_BIDS, time), bids(b) {}

    RemoveBidsEvent(time_t offs_sec, bidDB_t &b) 
      : Event(REMOVE_BIDS, offs_sec), bids(b) {}
    
    RemoveBidsEvent(bidDB_t &b) 
      : Event(REMOVE_BIDS), bids(b) {}

    bidDB_t *getBids()
    {
        return &bids;
    }
    
    int deleteBid(int uid)
    {
        int ret = 0;
        bidDBIter_t iter;
        
        for (iter=bids.begin(); iter != bids.end(); iter++) {
            if ((*iter)->getUId() == uid) {
                bids.erase(iter);
                ret++;
                break;
            }   
        }
          
        if (bids.empty()) {
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

class InsertBidAuctionEvent : public Event
{
  private:
	Bid * bid;
	string auctionSet;
	string auctionName;
	
  public:

    InsertBidAuctionEvent(struct timeval time, Bid *_bid, 
							string _auctionSet, string _auctionName) 
      : Event(ADD_BID_AUCTION, time), bid(_bid), auctionSet(_auctionSet),
		auctionName(_auctionName){}

     InsertBidAuctionEvent(time_t offs_sec, Bid *_bid, string _auctionSet, 
							string _auctionName) 
      : Event(ADD_BID_AUCTION, offs_sec), bid(_bid), auctionSet(_auctionSet),
		auctionName(_auctionName) {}

     InsertBidAuctionEvent(Bid *_bid, string _auctionSet, 
							string _auctionName ) 
      : Event(ADD_BID_AUCTION), bid(_bid), auctionSet(_auctionSet),
		auctionName(_auctionName) {}

	 Bid * getBid()
	 {
	     return bid;
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


class RemoveBidAuctionEvent : public Event
{
  private:
	Bid * bid;
	string auctionSet;
	string auctionName;
	
  public:

    RemoveBidAuctionEvent(struct timeval time, Bid *_bid, 
							string _auctionSet, string _auctionName) 
      : Event(REMOVE_BID_AUCTION, time), bid(_bid), auctionSet(_auctionSet),
		auctionName(_auctionName){}

     RemoveBidAuctionEvent(time_t offs_sec, Bid *_bid, string _auctionSet, 
							string _auctionName) 
      : Event(REMOVE_BID_AUCTION, offs_sec), bid(_bid), auctionSet(_auctionSet),
		auctionName(_auctionName) {}

     RemoveBidAuctionEvent(Bid *_bid, string _auctionSet, 
							string _auctionName ) 
      : Event(REMOVE_BID_AUCTION), bid(_bid), auctionSet(_auctionSet),
		auctionName(_auctionName) {}

	 Bid * getBid()
	 {
		 return bid;
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



//! add bids flags
const int ADD_BIDS_MAPI   = 0x1;

class AddBidsCtrlEvent : public CtrlCommEvent
{
  private:
    int type;
    char *buf;
    int len;

  public:

    AddBidsCtrlEvent(char *b, int l)
      : CtrlCommEvent(ADD_BIDS_CTRLCOMM), type(0), len(l) 
    {
        buf = new char[len+1];
        memcpy(buf, b, len+1);
          
    }

    ~AddBidsCtrlEvent()
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


class RemoveBidsCtrlEvent : public CtrlCommEvent
{
  private:
    string bid;

  public:
    
    RemoveBidsCtrlEvent(string b) 
      : CtrlCommEvent(REMOVE_BIDS_CTRLCOMM), bid(b) {}

    string getBid()
    {
        return bid;
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
