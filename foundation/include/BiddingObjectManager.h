
/*! \file   BiddingObjectManager.h

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
    biddingObject  db
    Code based on Netmate Implementation

	$Id: BiddingObjectManager.h 748 2015-07-23 12:09:00Z amarentes $
*/

#ifndef _BIDMANAGER_H_
#define _BIDMANAGER_H_


#include "stdincpp.h"
#include "Logger.h"
#include "Error.h"
#include "IdSource.h"
#include "ProcModuleInterface.h"
#include "AuctioningObjectManager.h"
#include "BiddingObject.h"
#include "BiddingObjectFileParser.h"
#include "MAPIBiddingObjectParser.h"
#include "EventScheduler.h"
#include "FieldDefManager.h"

namespace auction
{

// default flow idle timeout
const time_t FLOW_IDLE_TIMEOUT = 30;


//! index biddingObjects by time
typedef map<time_t, auctioningObjectDB_t>            	biddingObjectTimeIndex_t;
typedef map<time_t, auctioningObjectDB_t>::iterator  	biddingObjectTimeIndexIter_t;

typedef map<string, vector<int> >					auctionBidIndex_t;
typedef map<string, vector<int> >::iterator			auctionBidIndexIter_t;
typedef map<string, auctionBidIndex_t>				auctionSetBidIndex_t;
typedef map<string, auctionBidIndex_t>::iterator	auctionSetBidIndexIter_t;

/*! \short   manage adding/deleting of complete biddingObject  descriptions
  
  the BiddingObjectManager class allows to add and remove biddingObjects in the Auction
  core system. biddingObject  data are a set of ascii strings that are parsed
  and syntax checked by the BiddingObjectManager and then their respective
  settings are used to configure the other AuctionCore components. 
  The biddingObject  will then be stored in the bidDatabase inside the BiddingObjectManager
*/

class BiddingObjectManager : public AuctioningObjectManager
{
  private:


	//! index biddingObjects via AuctionSetId and Auction name.
	auctionSetBidIndex_t bidAuctionSetIndex;
	
    //! connection string to the database.
    string connectionDBStr;
    
	/*! \short add the biddingObject  name to the list of finished biddingObjects
       \arg \c b - object to store biddingObject  (source.name).
    */
    void storeBiddingObjectAsDone(BiddingObject  *b);
  
  public:

    /*! \short   construct and initialize a BiddingObjectManager object
        \arg \c fdname  		field definition file name
        \arg \c fvname  		field value definition name
        \arg \c connectionDB 	string to connect to the data base. If empty inactive DB management.
     */
    BiddingObjectManager(int domain, string fdname, string fvname, string connectionDB); //Ok

    //! destroy a BiddingObjectManager object
    ~BiddingObjectManager(); // Ok

	//! get biddingObjects by auction set and auction name
	vector<int> getBiddingObjects(string aset, string aname);

    //! parse XML biddingObjects from file 
    auctioningObjectDB_t *parseBiddingObjects(string fname); // Ok

    //! parse XML or Auction API biddingObjects from buffer
    auctioningObjectDB_t *parseBiddingObjectsBuffer(char *buf, int len);

    //! parse biddingObjects from ipap_message 
    auctioningObjectDB_t *parseMessage(ipap_message *messageIn, ipap_template_container *templates);

   
    /*! \short   add a biddingObject  list

        adding new biddingObjects to the Auction system will parse and syntax
        check the given biddingObject  specifications, lookup the biddingObject  database for
        already installed biddingObjects and store the biddingObject  into the database 

        \throws an Error exception if the given biddingObject  description is not
        syntactically correct or does not contain the mandatory fields
        or if a biddingObject  with the given identification is already present in the bidDatabase
    */
    void addAuctioningObjects(auctioningObjectDB_t *biddingObjects, EventScheduler *e);  

	//! add a single biddingObject
	void addBiddingObject(BiddingObject *b);

	void delBiddingObject(BiddingObject *r, EventScheduler *e);

	void delBiddingObject(int uid, EventScheduler *e);
	
	void delBiddingObjects(string sname, EventScheduler *e); 
	
	void delBiddingObject(string sname, string rname, EventScheduler *e);
	
	void delAuctioningObjects(auctioningObjectDB_t *biddingObjects, EventScheduler *e);
	

	/*! \short   get the ipap_message that contains all the bidding objects within
     * 			  the container. 

        \throws an Error exception if some field required is missing.
    */	
	ipap_message * get_ipap_message(BiddingObject *biddingObject,
									Auction *auction, 
									ipap_template_container *templates);

    /*! \short   get information from the auction manager

        these functions can be used to get information for a single auction object,
        or a set of auction objects
    */
    string getInfo(int uid);
    string getInfo(string sname, string rname);
    string getInfo(string sname);
    string getInfo();

    //! dump a AuctionManager object
    void dump( ostream &os );
	
};


//! overload for <<, so that a BiddingObjectManager object can be thrown into an iostream
ostream& operator<< ( ostream &os, BiddingObjectManager &rm );

} // namespace auction

#endif // _BIDMANAGER_H_
