
/*! \file   Bid.h

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
    Bids in the system - All concrete bids inherit from this class.

    $Id: Bid.h 748 2015-07-23 15:30:00Z amarentes $
*/

#ifndef _BID_H_
#define _BID_H_

#include "stdincpp.h"
#include "Logger.h"
#include "ConfigParser.h"
#include "Field.h"
#include "AuctionTimer.h"

namespace auction
{


//! Bid's states during lifecycle
typedef enum
{
    BS_NEW = 0,
    BS_VALID,
    BS_SCHEDULED,
    BS_ACTIVE,
    BS_DONE,
    BS_ERROR
} bidState_t;


//! execution list intervals.
typedef vector<interval_t>            bidIntervalList_t;
typedef vector<interval_t>::iterator  bidIntervalListIter_t;
typedef vector<interval_t>::const_iterator  bidIntervalListConstIter_t;

//! This bid applies to bid in this auction.
class bid_auction_t
{
  public:
	string auctionSet;
	string auctionName;
	int interval;
	int align;
	//! define the bid-auction running time properties
	time_t start;
	time_t stop;
	
	bid_auction_t(){}
	
	~bid_auction_t(){}
		
	string getId(){ return auctionSet + "." + auctionName; }
		
	bool operator==(const bid_auction_t &rhs);
		
	bool operator!=(const bid_auction_t &rhs);
	
};

//! element map (elementName, fieldlist)
typedef map<string, fieldList_t>            		elementList_t;
typedef map<string, fieldList_t>::iterator  		elementListIter_t;
typedef map<string, fieldList_t>::const_iterator  	elementListConstIter_t;

typedef map<string, bid_auction_t>						bidAuctionList_t;
typedef map<string, bid_auction_t>::iterator			bidAuctionListIter_t;
typedef map<string, bid_auction_t>::const_iterator		bidAuctionListConstIter_t;

class Bid
{
private:
    Logger *log; //!< link to global logger object
    int ch;      //!< logging channel number used by objects of this class

public:

	
    /*! \short   This function creates a new object instance. It recalculates intervals.
        \returns a new object instance.
    */
	Bid( string sname, string rname, elementList_t &e, bidAuctionList_t &ba );

	Bid( const Bid &rhs );

	~Bid();

    int getUId() 
    { 
        return uid;
    }
    
    void setUId(int nuid)
    {
        uid = nuid;
    }

		
    void setState(bidState_t s) 
    { 
        state = s;
    }

    bidState_t getState()
    {
        return state;
    }

    void setSetName(string _setName)
	{
		setName = _setName;
	}	

    string getSetName()
    {
        return setName;
    }

	void setBidName(string _bidName)
	{
		bidName = _bidName;
	}

    string getBidName()
    {
        return bidName;
    }


	string getInfo();
		

    /*! \short   get names and values (parameters) of configured elements
        \returns a pointer (link) to a list that contains the configured elements for this bid
    */
    inline elementList_t *getElements(){ return &elementList; }

	
    /*! \short   get set and name of all auctions to compete.
        \returns a pointer (link) to a list that contains the auctions for this bid
    */
	inline bidAuctionList_t *getAuctions(){ return &auctionList; }
	
	void deleteAuction(string aset, string aName);
	
	bool operator==(const Bid &rhs);
	
	bool operator!=(const Bid &rhs);
	
protected:
	
    //! unique bidID of this Rule instance (has to be provided)
    int uid;

    //! name of the rule for the external system calling the Auction Manager
    string bidName;

    //! name of the agent set this bid belongs to
    string setName;
   
	//! state of this rule
    bidState_t state;

    //! list of elements
    elementList_t elementList;
    
    //! list of auctions for this bid
    bidAuctionList_t auctionList;

};

}; // namespace auction

#endif // _BID_H_
