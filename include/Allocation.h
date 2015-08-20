
/*! \file   Allocation.h

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
    Allocations in the system - All concrete allocations inherit from this class.

    $Id: Allocation.h 748 2015-08-20 14:45:00Z amarentes $
*/

#ifndef _ALLOCATION_H_
#define _ALLOCATION_H_

#include "stdincpp.h"
#include "Logger.h"
#include "ConfigParser.h"
#include "ProcModuleInterface.h"


//! Allocation's states during lifecycle
typedef enum
{
    AS_NEW = 0,
    AS_SCHEDULED,
    AS_ACTIVE,
    AS_DONE,
    AS_ERROR
} allocationState_t;

//! execution interval definition
typedef struct 
{
    //! Allocation interval start
    time_t start;
    //! Allocation interval stop
    time_t stop;
} alloc_interval_t;



//! execution list intervals.
typedef vector<alloc_interval_t>            	  allocationIntervalList_t;
typedef vector<alloc_interval_t>::iterator  	  allocationIntervalListIter_t;
typedef vector<alloc_interval_t>::const_iterator  allocationIntervalListConstIter_t;


class Allocation
{

public:
	
    /*! \short   This function creates a new object instance. 
        \returns a new object instance.
    */
	Allocation( string aset, string aname, string bset, string bname );

	~Allocation();

    int getUId() 
    { 
        return uid;
    }
    
    void setUId(int nuid)
    {
        uid = nuid;
    }

    void setState(allocationState_t s) 
    { 
        state = s;
    }

    allocationState_t getState()
    {
        return state;
    }

    void setAuctionSet(string _set)
	{
		auctionSet = _set;
	}	

    string getAuctionSet()
    {
        return auctionSet;
    }

    void setAuctionName(string _name)
	{
		auctionName = _name;
	}	

    string getAuctionName()
    {
        return auctionName;
    }

	void setBidSet(string _bidSet)
	{
		bidSet = _bidSet;
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


	string getInfo();
		

    /*! \short   get names and values of the allocation's results
        \returns a pointer (link) to a list that contains the results elements for this allocation
    */
    fieldList_t *getFields();
        
    /*! \short   get intervals setup for the allocation
        \returns a pointer (link) to a list that contains the intervals configured for this allocation
    */
    allocationIntervalList_t * getIntervals();
	
	int getNumIntervals()
	{
		return (int) intervals.size();
	}
	
	int getNumFields()
	{
		return (int) fields.size();
	}
	
protected:
	
    //! unique bidID of this Rule instance (has to be provided)
    int uid;
   
	//! state of this rule
    allocationState_t state;

	//! Auction set associated to this allocation
	string auctionSet;  

	//! Auction name associated to this allocation
	string auctionName; 

	//! Bid set associated to this allocation
	string bidSet;		

	//! Bid name associated to this allocation
	string bidName;     

	//! Allocation fields, two typical fields are quantity and price.
	fieldList_t fields; 

	//! Allocation intervals assigned.
	allocationIntervalList_t intervals;


};

#endif // _BID_H_
