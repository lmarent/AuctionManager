
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

typedef struct
{
	time_t start;
	time_t stop;

} bidInterval_t;

//! Intervals defined for the bid.
typedef vector< pair<time_t, bidInterval_t> > 					bidIntervalList_t;
typedef vector< pair<time_t, bidInterval_t> >::iterator			bidIntervalListIter_t;
typedef vector< pair<time_t, bidInterval_t> >::const_iterator	bidIntervalListConstIter_t;


//! element map (elementName, fieldlist).
typedef map<string, fieldList_t>            		elementList_t;
typedef map<string, fieldList_t>::iterator  		elementListIter_t;
typedef map<string, fieldList_t>::const_iterator  	elementListConstIter_t;


//! option vector (optionName, fieldlist), options must be ordered.
typedef vector< pair<string, fieldList_t> >            			optionList_t;
typedef vector< pair<string, fieldList_t> >::iterator  			optionListIter_t;
typedef vector< pair<string, fieldList_t> >::const_iterator  	optionListConstIter_t;

class Bid
{
private:
    Logger *log; //!< link to global logger object
    int ch;      //!< logging channel number used by objects of this class

public:

	
    /*! \short   This function creates a new object instance. It recalculates intervals.
        \returns a new object instance.
    */
	Bid( string auctionSet, string auctionName, string bidSet, string bidName, 
			elementList_t &elements, optionList_t &options );

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

    bidState_t getBidState()
    {
        return state;
    }

    void setBidSet(string _bidset)
	{
		bidSet = _bidset;
	}	

    string getBidSet()
    {
        return bidSet;
    }

	void setBidName(string _bidName)
	{
		bidName = _bidName;
	}

    string getBidName()
    {
        return bidName;
    }


    void setAuctionSet(string _auctionset)
	{
		auctionSet = _auctionset;
	}	

    string getAuctionSet()
    {
        return auctionSet;
    }

	void setAuctionName(string _auctionName)
	{
		auctionName = _auctionName;
	}

    string getAuctionName()
    {
        return auctionName;
    }


	string getInfo();
		

    /*! \short   get names and values (parameters) of configured elements
        \returns a pointer (link) to a list that contains the configured elements for this bid
    */
    inline elementList_t *getElements(){ return &elementList; }

	inline optionList_t *getOptions() {return &optionList; }
	
	bool operator==(const Bid &rhs);
	
	bool operator!=(const Bid &rhs);
	
	//! get a value by name from the element attributes 
	field_t getElementVal(string elementName, string name);
	
	//! get a value by name from the misc rule attributes
    field_t getOptionVal(string optionName, string name);

	//! Calculates intervals associated to bid.
	void calculateIntervals(time_t now, bidIntervalList_t *list);
    
       
	
protected:
	
    //! unique bidID of this Rule instance (has to be provided)
    int uid;

	//! state of this bid
    bidState_t state;

    //! set of the auction that this bid belongs to 
    string auctionSet;
    
    //! name of the auction that this bid belongs to 
    string auctionName;

    //! name of the set that this bid belongs to
    string bidSet;

    //! name of the rule for the external system calling the Auction Manager
    string bidName;
   
    //! list of elements
    elementList_t elementList;
    
    //! List of options associated with the bid.
    //! Important options are start and stop, which define rul
    optionList_t optionList;
    
};

}; // namespace auction

#endif // _BID_H_
