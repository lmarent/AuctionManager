
/*! \file   AuctionManager.h

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
    Auction db
    Code based on Netmate Implementation

	$Id: AuctionManager.h 748 2015-08-04 13:31:00Z amarentes $
*/

#ifndef _AUCTION_MANAGER_H_
#define _AUCTION_MANAGER_H_


#include "stdincpp.h"
#include "Logger.h"
#include "Error.h"
#include "AuctionIdSource.h"
#include "Auction.h"
#include "AuctionFileParser.h"
#include "EventScheduler.h"
#include "MAPIAuctionParser.h"

namespace auction
{

// AuctionDB definition is currently in AuctionFileParser.h

// index by set id and name
typedef map<string, int>            		auctionIndex_t;
typedef map<string, int>::iterator  		auctionIndexIter_t;
typedef map<string, auctionIndex_t>             auctionSetIndex_t;
typedef map<string, auctionIndex_t>::iterator   auctionSetIndexIter_t;

//! list of done auctions
typedef list<Auction*>            auctionDone_t;
typedef list<Auction*>::iterator  auctionDoneIter_t;

//! index auctions by time
typedef map<time_t, auctionDB_t>            auctionTimeIndex_t;
typedef map<time_t, auctionDB_t>::iterator  auctionTimeIndexIter_t;

//! compare two export definition structs
struct lttexp
{
    bool operator()(const procdef_t e1, const procdef_t e2) const
    {
      if  ((e1.interval.interval < e2.interval.interval) ||
           (e1.interval.align < e2.interval.align) ||
           (strcmp(e1.procname.c_str(), e2.procname.c_str()))) {
          return 1;
      } else {
          return 0;
      }
    }
};

//! auctions indexed by interval and execution modules struct
typedef map<procdef_t, auctionDB_t, lttexp>            auctionIntervalsIndex_t;
typedef map<procdef_t, auctionDB_t, lttexp>::iterator  auctionIntervalsIndexIter_t;


/*! \short   manage adding/deleting of complete auction descriptions
  
  the AuctionManager class allows to add and remove auctions in the Auction
  core system. Auction data are a set of ascii strings that are parsed
  and syntax checked by the AuctionManager and then their respective
  settings are used to configure the other AuctionCore components. 
  The Auction will then be stored in the AuctionDatabase inside the AuctionManager
*/

class AuctionManager
{
  private:

    Logger *log;
    int ch; //!< logging channel number used by objects of this class

    //!< number of bids in the database
    int auctions;

    //! index to auctions via setID and name
    auctionSetIndex_t auctionSetIndex;

    //! stores all auctions indexed by setID, bidID
    auctionDB_t  auctionDB;

    //! list with auctions done
    auctionDone_t auctionDone;

	//! field definitions
    fieldDefList_t fieldDefs;

    //! field values
    fieldValList_t fieldVals;

	ipap_message *message;		///< Empty message for creating copies of it.

    // name of field def and field vals files
    string fieldDefFileName, fieldValFileName;
		
    // pool of unique bid ids
    AuctionIdSource idSource;

    /* \short add the auction name to the list of finished bids

       \arg \c auctionname - name of the finished auction (source.name)
    */
    void storeAuctionAsDone(Auction *a);


    //! load field definitions
    void loadFieldDefs(string fname);

	//! load field value definitions
    void loadFieldVals(string fname);


  public:

    int getNumAuctions() //Ok
    { 
        return auctions; 
    }

    string getInfo(int uid) // Ok
    {
        return getInfo(getAuction(uid)); 
    }

    /*! \short   construct and initialize a AuctionManager object
     */
    AuctionManager( string fdname, string fvname); // Ok

    //! destroy a AuctionManager object
    ~AuctionManager(); //Ok

     /*! \short   lookup the auction info data for a given auctionId or name

        lookup the database of auctions for a specific auction
        and return a link (pointer) to the auction data associated with it
        do not store this pointer, its contents will be destroyed upon auction deletion. 
        do not free this pointer as it is a link to the auction and not a copy.
    
        \arg \c uId - unique auction id
    */
    Auction *getAuction(int uid); // Ok

    //! get auction rname from auctionset sname 
    Auction *getAuction(string sname, string rname); 

    //! get all auctions in auctionset with name sname 
    auctionIndex_t *getAuctions(string sname);

    //! get all bids, creates a nuew vector with the same pointers to auctions,
	//! so it does not require to free memory. 
    auctionDB_t getAuctions(); //Ok

    //! parse XML auctions from file 
    auctionDB_t *parseAuctions(string fname, ipap_template_container *templates);  // Ok

    //! parse XML or Auction API bids from buffer
    auctionDB_t *parseAuctionsBuffer(char *buf, int len, ipap_template_container *templates);
   
    //! parse auctions from ipap_message 
    auctionDB_t *parseAuctionsMessage(ipap_message *messageIn, ipap_template_container *templates);
   
    /*! \short   add a auction description 

        adding new auctions to the Auction system will parse and syntax
        check the given auction specifications, lookup the auction database for
        already installed auctions and store the auction into the database. 
        This function assigns the UID for all auctions given.

        \throws an Error exception if the given auction description is not
        syntactically correct or does not contain the mandatory fields
        or if a auction with the given identification is already present in the AuctionDatabase
    */
    void addAuctions(auctionDB_t *auctions, EventScheduler *e);  //Ok

    //! add a single auction, the methods assigns the UID for the auction.
    void addAuction(Auction *b); // Ok

    /*! \short   delete a Auction description 

        deletion of a auction will parse and syntax check the
        identification string, test the presence of the given auction
        in the auctionDatabase, remove the auction from the auctionDatabase

        \throws an Error exception if the given auction identification is not
        syntactically correct or does not contain the mandatory fields  or 
        if a auction with the given identification is currently not present 
        in the auctionDatabase
    */
    void delAuction(int uid, EventScheduler *e); // Ok
    void delAuction(string sname, string rname, EventScheduler *e); // Ok
    void delAuctions(string sname, EventScheduler *e); // Ok
    void delAuction(Auction *a, EventScheduler *e); // Ok
    void delAuctions(auctionDB_t *auctions, EventScheduler *e); //Ok
   
    /*! \short   get information from the auction manager

        these functions can be used to get information for a single auction,
        or a set of auctions or all auctions
    */
    string getInfo(void);
    string getInfo(Auction *a);
    string getInfo(string sname, string rname);
    string getInfo(string sname);

    //! dump a AuctionManager object
    void dump( ostream &os );
};


//! overload for <<, so that a AuctionManager object can be thrown into an iostream
ostream& operator<< ( ostream &os, AuctionManager &rm );

}; // namespace auction

#endif // _AUCTION_MANAGER_H_
