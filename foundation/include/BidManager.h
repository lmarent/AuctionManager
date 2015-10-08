
/*! \file   BidManager.h

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
    bid db
    Code based on Netmate Implementation

	$Id: BidManager.h 748 2015-07-23 12:09:00Z amarentes $
*/

#ifndef _BIDMANAGER_H_
#define _BIDMANAGER_H_


#include "stdincpp.h"
#include "Logger.h"
#include "Error.h"
#include "BidIdSource.h"
#include "Bid.h"
#include "BidFileParser.h"
#include "MAPIBidParser.h"
#include "EventScheduler.h"

namespace auction
{

// default flow idle timeout
const time_t FLOW_IDLE_TIMEOUT = 30;


// RuleDB definition is currently in RuleFileParser.h

// index by set id and name
typedef map<string, int>            		bidIndex_t;
typedef map<string, int>::iterator  		bidIndexIter_t;
typedef map<string, bidIndex_t>             bidSetIndex_t;
typedef map<string, bidIndex_t>::iterator   bidSetIndexIter_t;

//! list of done Bids
typedef list<Bid*>            bidDone_t;
typedef list<Bid*>::iterator  bidDoneIter_t;

//! index bids by time
typedef map<time_t, bidDB_t>            bidTimeIndex_t;
typedef map<time_t, bidDB_t>::iterator  bidTimeIndexIter_t;

typedef map<string, vector<int> >					auctionBidIndex_t;
typedef map<string, vector<int> >::iterator			auctionBidIndexIter_t;
typedef map<string, auctionBidIndex_t>				auctionSetBidIndex_t;
typedef map<string, auctionBidIndex_t>::iterator	auctionSetBidIndexIter_t;

/*! \short   manage adding/deleting of complete bid descriptions
  
  the BidManager class allows to add and remove bids in the Auction
  core system. Bid data are a set of ascii strings that are parsed
  and syntax checked by the BidManager and then their respective
  settings are used to configure the other AuctionCore components. 
  The bid will then be stored in the bidDatabase inside the BidManager
*/

class BidManager
{
  private:

    Logger *log;
    int ch; //!< logging channel number used by objects of this class

    //!< number of bids in the database
    int bids;

    //! index to bids via setID and name
    bidSetIndex_t bidSetIndex;

	//! index bids via AuctionSetId and Auction name.
	auctionSetBidIndex_t bidAuctionSetIndex;

    //! stores all bids indexed by setID, bidID
    bidDB_t  bidDB;

    //! list with bids done
    bidDone_t bidDone;

	//! field definitions
    fieldDefList_t fieldDefs;

    //! field values
    fieldValList_t fieldVals;

    // name of field def and field vals files
    string fieldDefFileName, fieldValFileName;

    //! load filter definitions
    void loadFieldDefs(string fname);

    //! load filter value definitions
    void loadFieldVals(string fname);


    // pool of unique bid ids
    BidIdSource idSource;

    /* \short add the bid name to the list of finished bids

       \arg \c bidname - name of the finished bid (source.name)
    */
    void storeBidAsDone(Bid *r);

  public:

    int getNumBids() 
    { 
        return bids; 
    }

    string getInfo(int uid)
    {
        return getInfo(getBid(uid)); 
    }

    /*! \short   construct and initialize a BidManager object
        \arg \c fdname  field definition file name
        \arg \c fvname  field value definition name
     */
    BidManager(string fdname, string fvname); //Ok

    //! destroy a BidManager object
    ~BidManager(); // Ok

     /*! \short   lookup the bid info data for a given BidId or name

        lookup the database of bids for a specific bid
        and return a link (pointer) to the Bid data associated with it
        do not store this pointer, its contents will be destroyed upon bid deletion. 
        do not free this pointer as it is a link to the Bid and not a copy.
    
        \arg \c bidId - unique bid id
    */
    Bid *getBid(int uid);

    //! get bid rname from bidset sname 
    Bid *getBid(string sname, string rname); //Ok

    //! get all bids in bidset with name sname 
    bidIndex_t *getBids(string sname);

	//! get bids by auction set and auction name
	vector<int> getBids(string aset, string aname);

    //! get all bids
    bidDB_t getBids();

    //! parse XML bids from file 
    bidDB_t *parseBids(string fname); // Ok

    //! parse XML or Auction API bids from buffer
    bidDB_t *parseBidsBuffer(char *buf, int len);

    //! parse bids from ipap_message 
    bidDB_t *parseBidsMessage(ipap_message *messageIn, ipap_message *messageOut);

   
    /*! \short   add a bid list

        adding new bids to the Auction system will parse and syntax
        check the given bid specifications, lookup the bid database for
        already installed bids and store the bid into the database 

        \throws an Error exception if the given bid description is not
        syntactically correct or does not contain the mandatory fields
        or if a bid with the given identification is already present in the bidDatabase
    */
    void addBids(bidDB_t *bids, EventScheduler *e);  

    //! add a single bid
    void addBid(Bid *b); //ok

    //! activate/execute bids
    void activateBids(bidDB_t *bids, EventScheduler *e);

    /*! \short   delete a Bid description 

        deletion of a bid will parse and syntax check the
        identification string, test the presence of the given bid
        in the bidDatabase, remove the bid from the bidDatabase

        \throws an Error exception if the given bid identification is not
        syntactically correct or does not contain the mandatory fields  or 
        if a bid with the given identification is currently not present 
        in the bidDatabase
    */
    void delBid(int uid, EventScheduler *e); //ok
    void delBid(string rname, string sname, EventScheduler *e); // ok
    void delBids(string sname, EventScheduler *e);
    void delBid(Bid *r, EventScheduler *e);
    void delBids(bidDB_t *bids, EventScheduler *e); //Ok
   
    /*! \short   get information from the bid manager

        these functions can be used to get information for a single bid,
        or a set of bids or all bids
    */
    string getInfo(void);
    string getInfo(Bid *r);
    string getInfo(string sname, string rname);
    string getInfo(string sname);

    /*! \short   delete a relationship between a bid and an auction by id.
     * 			 if the bid does not have any other relationship with another
     * 			 auction, then the manager triggers the bid remove.

    */
	void delBidAuction(string auctionSet, string auctionName, int uid, EventScheduler *e);

    /*! \short   delete a relationship between a bid and an auction by set and name
     * 			 if the bid does not have any other relationship with another
     * 			 auction, then the manager triggers the bid remove.

    */
	void delBidAuction(string auctionSet, string auctionName, 
					   string bset, string bname, EventScheduler *e);

    //! dump a BidManager object
    void dump( ostream &os );
};


//! overload for <<, so that a BidManager object can be thrown into an iostream
ostream& operator<< ( ostream &os, BidManager &rm );

}; // namespace auction

#endif // _BIDMANAGER_H_
