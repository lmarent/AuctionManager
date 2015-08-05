
/*!  \file   Auction.h

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
    Defines the auction object.
    Code based on Netmate Implementation

    $Id: Auction.h 748 2015-08-04 9:46:00 amarentes $
*/

#ifndef _AUCTION_H_
#define _AUCTION_H_

#include "stdincpp.h"
#include "Logger.h"
#include "AuctionTimer.h"
#include "ConfigParser.h"

//! rule states during lifecycle
typedef enum
{
    AS_NEW = 0,
    AS_VALID,
    AS_SCHEDULED,
    AS_ACTIVE,
    AS_DONE,
    AS_ERROR
} AuctionState_t;

//! Algorith to execute for the auction process.
typedef struct
{
    string name;
    int defaultAct; //! 1 True, 0 False.
    configItemList_t conf;
} action_t;

//! execution interval definition
typedef struct 
{
    //! execution interval
    unsigned long interval;
    //! align yes/no
    int align;
} interval_t;

//! compare two interval structs
struct lttint
{
    bool operator()(const interval_t i1, const interval_t i2) const
    {
      if  ((i1.interval < i2.interval) ||
           (i1.align < i2.align)) {
          return 1;
      } else {
          return 0;
      }
    }
};

//! execution list intervals.
typedef list<interval_t>            intervalList_t;
typedef list<interval_t>::iterator  intervalListIter_t;
typedef list<interval_t>::const_iterator  intervalListConstIter_t;


//! action list (only push_back & sequential access)
typedef list<action_t>            actionList_t;
typedef list<action_t>::iterator  actionListIter_t;
typedef list<action_t>::const_iterator  actionListConstIter_t;

//! misc list (random access based on name required)
typedef map<string,configItem_t>            miscList_t;
typedef map<string,configItem_t>::iterator  miscListIter_t;
typedef map<string,configItem_t>::const_iterator  miscListConstIter_t;

class Auction
{
  private:

    Logger *log; //!< link to global logger object
    int ch;      //!< logging channel number used by objects of this class
    
    //! define the rules running time properties
    time_t start;
    time_t stop;

	//! define the execution intervals.
	intervalList_t	intervals;

    //! unique auctionID of this auction instance (has to be provided)
    int uid;

    //! state of this auction
    AuctionState_t state;

    //! name of the auction by convention this must be either: <name> or <resource>.<id>
    string auctionName;

    //! parts of auction name for efficiency
    string resource;
    string id;

    //! name of the auction set this auction belongs to
    string setName;
	
	//! Execution method to be called everytime that the auction is timeout.
	action_t action;

    //! list of misc stuff (start, stop, duration etc.)
    miscList_t miscList;

    /*! \short   parse identifier format 'sourcename.rulename'

        recognizes dor (.) in task identifier and saves sourcename and 
        rulename to the new malloced strings source and rname
    */
    void parseAuctionName(string rname);

    //! parse time string
    time_t parseTime(string timestr);

    //! get a value by name from the misc rule attriutes
    string getMiscVal(string name);

  public:
    
    void setState(AuctionState_t s) 
    { 
        state = s;
    }

    AuctionState_t getState()
    {
        return state;
    }

    int getUId() 
    { 
        return uid;
    }
    
    void setUId(int nuid)
    {
        uid = nuid;
    }
    
    string getSetName()
    {
        return setName;
    }

    string getAuctionName()
    {
        return auctionName;
    }
    
    string getAUctionID()
    {
		return id;
	}
    
    string getAuctionResource()
    {
        return resource;
    }
    
    time_t getStart()
    {
        return start;
    }
    
    time_t getStop()
    {
        return stop;
    }
    
    intervalList_t *getIntervals()
    {
        return &intervals;
    }
            
    /*! \short   construct and initialize a Auction object
        \arg \c now   current timestamp
        \arg \c sname   auction set name
        \arg \c s  rname  auction name
        \arg \c a  action
        \arg \c m  list of misc parameters
    */
    Auction(unsigned short _uid, time_t now, string sname, string rname, action_t &a,
    	  miscList_t &m);

	/*! \short  construct an auction from another auction
	  	\arg \c rhs auction to copy from
	 */ 	
	Auction(const Auction &rhs);

    //! destroy a Auction object
    ~Auction();
   
    /*! \short   get names and values (parameters) of configured actions
        \returns a pointer (link) to an object that contains the configured action for this auction
    */
    action_t *getAction();

    /*! \short   get names and values (parameters) of misc. attributes

        \returns a pointer (link) to a ParameterSet object that contains the 
                 miscanellenous attributes of a configured auction
    */
    miscList_t *getMisc();

    //! dump a Auction object
    void dump( ostream &os );

    //! get rule info string
    string getInfo(void);

};

//! overload for <<, so that a Auction object can be thrown into an iostream
ostream& operator<< ( ostream &os, Auction &ai );	

#endif // _AUCTION_H_
