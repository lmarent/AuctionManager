
/*! \file   AllocationManager.h

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
    allocation database
    Code based on Netmate Implementation

	$Id: AllocationManager.h 748 2015-08-20 16:59:00Z amarentes $
*/

#ifndef _ALLOCATIONMANAGER_H_
#define _ALLOCATIONMANAGER_H_


#include "stdincpp.h"
#include "Logger.h"
#include "Error.h"
#include "AllocationIdSource.h"
#include "Allocation.h"
#include "ProcModuleInterface.h"
#include "EventScheduler.h"


// default flow idle timeout
const time_t FLOW_IDLE_TIMEOUT = 30;


// AllocationDB definition is currently in ProcModuleInterface.h

// index by set and name
typedef map<string, vector<int> >            	   allocationIndex_t;
typedef map<string, vector<int> >::iterator  	   allocationIndexIter_t;
typedef map<string, allocationIndex_t>             allocationSetIndex_t;
typedef map<string, allocationIndex_t>::iterator   allocationSetIndexIter_t;

//! list of done allocation
typedef list<Allocation*>            allocationDone_t;
typedef list<Allocation*>::iterator  allocationDoneIter_t;

//! index allocations by time
typedef map<time_t, allocationDB_t>            bidTimeIndex_t;
typedef map<time_t, allocationDB_t>::iterator  bidTimeIndexIter_t;


/*! \short   manage adding/deleting of complete allocation descriptions
  
  the AllocationManager class allows to add and remove allocations in 
  the auction core system. Allocation data are a set of ascii strings 
  that are parsed and syntax checked by the AllocationManager and 
  then their respective settings are used to configure sessions. 
*/

class AllocationManager
{
  private:

    Logger *log;
    int ch; //!< logging channel number used by objects of this class

    //!< number of allocations in the database
    int allocations;

    //! index to allocation via bid setID and name
    allocationSetIndex_t bidSetIndex;

    //! index to allocation via auction setID and name
    allocationSetIndex_t auctionSetIndex;

    //! stores all allocations indexed by allocationID
    allocationDB_t  allocationDB;

    //! list with allocations done
    allocationDone_t allocationDone;

	//! filter definitions
    fieldDefList_t fieldDefs;

    //! name of field defs file.
    string fieldDefFileName

    //! load field definitions
    void loadFieldDefs(string fname);

    //! pool of unique allocation ids
    AllocationIdSource idSource;

    /*! \short add the allocation to the list of finished allocations
        \arg \a allocation to store.
    */
    void storeBidAsDone(Allocation *r);

  public:

    int getNumAllocations() 
    { 
        return allocations; 
    }

    string getInfo(int uid)
    {
        return getInfo(getAllocation(uid)); 
    }

    /*! \short   construct and initialize a AllocationManager object
        \arg \c fdname  field definition file name
     */
    AllocationManager(string fdname);

    //! destroy a AllocationManager object
    ~AllocationManager();

     /*! \short   lookup the allocation info data for a given AllocationId

        lookup the database of allocations for a specific allocation
        and return a link (pointer) to the Allocation data associated with it
        do not store this pointer, its contents will be destroyed upon allocation deletion. 
        do not free this pointer as it is a link to the Allocation and not a copy.
    
        \arg \c uid - unique allocation id
    */
    Allocation *getAllocation(int uid);

    /*! \short get allocation by auction and bid set and name
         \arg \c aset - auction set 
         \arg \c aname - auction name 
         \arg \c bset - bid set
         \arg \c bname - bid name
    */
    Allocation *getAllocaton(string aset, string aname, string bset, string bname);

    /*! \short  get all bids in bidset with name sname 
         \arg \c bset - bid set
         \arg \c bname - bid name
    */
    bidIndex_t *getAllocationsByBid(string bset, string bname);

    //! get all allocations
    allocationDB_t getAllocations();

   
    /*! \short   adds a allocations 

        adding new allocations to the Auction system, it will parse and syntax
        check the given allocation specifications, lookup the allocation database for
        already installed allocations and store the allocation into the database 

        \throws an Error exception if the given allocation description is not
        syntactically correct or does not contain the mandatory fields
        or if a allocation with the given identification is already 
        present in the allocation database
    */
    void addAllocations(allocationDB_t *allocations, EventScheduler *e);  

    //! add a single allocation
    void addAllocation(Allocation *b); 

    //! activate/execute allocations
    void activateAllocations(allocationDB_t *allocations, EventScheduler *e);

    /*! \short   delete an allocation description 

        deletion of an allocation will parse and syntax check the
        identification string, test the presence of the given allocation
        in the allocation database, and remove the allocation from database

        \throws an Error exception if the given allocation identification is not
        syntactically correct or does not contain the mandatory fields  or 
        if an allocation with the given identification is currently not present 
        in the database
    */
    void delBid(int uid, EventScheduler *e);

    //! delete all allocations from a specific bid.
    void delBidAllocations(string bset, string bname, EventScheduler *e);
    
    //! delete all allocations from a specific auction.
    void delAuctionAllocations(string aset, string aname, EventScheduler *e);

    //! delete and allocation.
    void delAllocation(Allocation *a, EventScheduler *e);
        
    //! delete all allocation included in the list given as parameter.
    void delAllocations(allocationDB_t *allocations, EventScheduler *e);
   
    /*! \short   get information from the allocation manager

        these functions can be used to get information for a single allocation,
        or all allocations
    */
    string getInfo(void);
    string getInfo(Allocation *r);

    //! dump a AllocationManager object
    void dump( ostream &os );
};


//! overload for <<, so that a BidManager object can be thrown into an iostream
ostream& operator<< ( ostream &os, AllocationManager &rm );


#endif // _ALLOCATIONMANAGER_H_
