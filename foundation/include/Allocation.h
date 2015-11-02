
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
#include "AuctioningObject.h"

namespace auction
{



//! execution interval definition
typedef struct 
{
    //! Allocation interval start
    time_t start;
    //! Allocation interval stop
    time_t stop;
} alloc_interval_t;



//! execution list intervals.
typedef std::list<alloc_interval_t>            	  allocationIntervalList_t;
typedef std::list<alloc_interval_t>::iterator  	  allocationIntervalListIter_t;
typedef std::list<alloc_interval_t>::const_iterator  allocationIntervalListConstIter_t;


class Allocation : public AuctioningObject
{

public:
	
    /*! \short   This function creates a new object instance. 
        \returns a new object instance.
    */
	Allocation( string aset, string aname, string bset, string bname, 
				string allset, string allname, fieldList_t &f, 
				allocationIntervalList_t &alloc_inter );

	~Allocation();

    inline void setAuctionSet(string _set){ auctionSet = _set; }	

    inline string getAuctionSet(){ return auctionSet; }

    inline void setAuctionName(string _name){ auctionName = _name; }	

    inline string getAuctionName(){ return auctionName; }

	inline void setBidSet(string _bidSet){ bidSet = _bidSet; }

    inline string getBidSet(){ return bidSet; }

	inline void setBidName(string _bidName){ bidName = _bidName; }

    inline string getBidName(){ return bidName; }
	
	inline void setAllocationSet(string _allocationSet){ allocationSet = _allocationSet; }
	
	inline string getAllocationSet(){ return allocationSet; }

	inline void setAllocationName(string _allocationName){ 	allocationName = _allocationName; }

	inline string getAllocationName(){ return allocationName; }
	
	string getAuctionIpApId();
	
	string getBidIpApId();
	
	string getIpApId(int domain);
	
	string getInfo();
		

    /*! \short   get names and values of the allocation's results
        \returns a pointer (link) to a list that contains the results elements for this allocation
    */
    inline fieldList_t *getFields(){ return &fields; }
        
    /*! \short   get intervals setup for the allocation
        \returns a pointer (link) to a list that contains the intervals configured for this allocation
    */
    inline allocationIntervalList_t * getIntervals() { return &intervals; }
	
	int getNumIntervals() { return intervals.size(); }
	
	int getNumFields() { return fields.size(); }
		
protected:
	   

	//! Auction set associated to this allocation
	string auctionSet;  

	//! Auction name associated to this allocation
	string auctionName; 

	//! Bid set associated to this allocation
	string bidSet;		

	//! Bid name associated to this allocation
	string bidName;     

	//! Allocation Set 
	string allocationSet;
	
	//! Allocation Name
	string allocationName;

	//! Allocation fields, two typical fields are quantity and price.
	fieldList_t fields; 

	//! Allocation intervals assigned.
	allocationIntervalList_t intervals;


};

}; // namespace auction



#endif // _ALLOCATION_H_
