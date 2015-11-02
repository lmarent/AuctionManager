
/*! \file   BiddingObject.h

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
    BiddingObjects in the system - A bidding object can be a bid, ask or allocation.

    $Id: BiddingObject.h 748 2015-07-23 15:30:00Z amarentes $
*/

#ifndef _BIDDINGOBJECT_H_
#define _BIDDINGOBJECT_H_

#include "stdincpp.h"
#include "Logger.h"
#include "ConfigParser.h"
#include "Field.h"
#include "IpAp_template.h"
#include "AuctionTimer.h"
#include "AuctioningObject.h"

namespace auction
{


typedef struct
{
	time_t start;
	time_t stop;

} biddingObjectInterval_t;

//! Intervals defined for the BiddingObject.
typedef vector< pair<time_t, biddingObjectInterval_t> > 					biddingObjectIntervalList_t;
typedef vector< pair<time_t, biddingObjectInterval_t> >::iterator			biddingObjectIntervalListIter_t;
typedef vector< pair<time_t, biddingObjectInterval_t> >::const_iterator		biddingObjectIntervalListConstIter_t;


//! element map (elementName, fieldlist).
typedef map<string, fieldList_t>            		elementList_t;
typedef map<string, fieldList_t>::iterator  		elementListIter_t;
typedef map<string, fieldList_t>::const_iterator  	elementListConstIter_t;


//! option vector (optionName, fieldlist), options must be ordered.
typedef vector< pair<string, fieldList_t> >            			optionList_t;
typedef vector< pair<string, fieldList_t> >::iterator  			optionListIter_t;
typedef vector< pair<string, fieldList_t> >::const_iterator  	optionListConstIter_t;

class BiddingObject : public AuctioningObject
{

public:

	
    /*! \short   This function creates a new object instance. It recalculates intervals.
        \returns a new object instance.
    */
	BiddingObject(  string auctionSet, string auctionName, string BiddingObjectSet, string BiddingObjectName, 
				    ipap_object_type_t _type, elementList_t &elements, optionList_t &options );

	BiddingObject( const BiddingObject &rhs );

	~BiddingObject();

    inline void setBiddingObjectSet(string _BiddingObjectset){ 	BiddingObjectSet = _BiddingObjectset; }	

    inline string getBiddingObjectSet() { return BiddingObjectSet; }

	inline void setBiddingObjectName(string _BiddingObjectName){ BiddingObjectName = _BiddingObjectName; }

    inline string getBiddingObjectName(){ return BiddingObjectName; }

    inline void setAuctionSet(string _auctionset){ auctionSet = _auctionset; }	

    inline string getAuctionSet(){ return auctionSet; }

	inline void setAuctionName(string _auctionName) { auctionName = _auctionName; }

    inline string getAuctionName(){ return auctionName; }

    /*! \short   get the Id to be used when transfering the object in a ipap_message
     *   \arg domain - domain to use in case that the BiddingObject does not have a group.
    */	
	string getIpApId(int domain);

    /*! \short   get the Id for the auction that this BiddingObject belongs for using when transfer the BiddingObject in a ipap_message
    */	
	string getAuctionIpAPId();
	
	string getInfo();
		

    /*! \short   get names and values (parameters) of configured elements
        \returns a pointer (link) to a list that contains the configured elements for this BiddingObject
    */
    inline elementList_t *getElements(){ return &elementList; }

	inline optionList_t *getOptions() {return &optionList; }
	
	bool operator==(const BiddingObject &rhs);
	
	bool operator!=(const BiddingObject &rhs);
	
	//! get a value by name from the element attributes 
	field_t getElementVal(string elementName, string name);
	
	//! get a value by name from the misc rule attributes
    field_t getOptionVal(string optionName, string name);

	//! Calculates intervals associated to BiddingObject.
	void calculateIntervals(time_t now, biddingObjectIntervalList_t *list);
    
    //! Get the type of bidding object
    inline ipap_object_type_t getType() { return biddingObjectType; }
	
	//! Set the type of the bidding object
	inline void setType(ipap_object_type_t _type) { biddingObjectType = _type; }
	
protected:
	

    //! set of the auction that this BiddingObject belongs to 
    string auctionSet;
    
    //! name of the auction that this BiddingObject belongs to 
    string auctionName;

    //! name of the set that this BiddingObject belongs to
    string BiddingObjectSet;

    //! name of the rule for the external system calling the Auction Manager
    string BiddingObjectName;
   
    //! Bidding object type
    ipap_object_type_t biddingObjectType;
   
    //! list of elements
    elementList_t elementList;
    
    //! List of options associated with the BiddingObject.
    //! Important options are start and stop, which define rul
    optionList_t optionList;
    
};

}; // namespace auction

#endif // _BIDDINGOBJECT_H_
