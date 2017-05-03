
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
#include "IdSource.h"
#include "Auction.h"
#include "AuctioningObjectManager.h"
#include "AuctionFileParser.h"
#include "EventScheduler.h"
#include "MAPIAuctionParser.h"
#include "FieldDefManager.h"

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
typedef map<time_t, auctioningObjectDB_t>            auctionTimeIndex_t;
typedef map<time_t, auctioningObjectDB_t>::iterator  auctionTimeIndexIter_t;

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


typedef set<int> 		    auctionSet_t;
typedef set<int>::iterator  auctionSetIter_t;

/*! \short   manage adding/deleting of complete auction descriptions
  
  the AuctionManager class allows to add and remove auctions in the Auction
  core system. Auction data are a set of ascii strings that are parsed
  and syntax checked by the AuctionManager and then their respective
  settings are used to configure the other AuctionCore components. 
  The Auction will then be stored in the AuctionDatabase inside the AuctionManager
*/

class AuctionManager : public AuctioningObjectManager
{

  private:
    
    //! If true the process request is created in the auction creation time, 
    //! otherwise it is created when during the auction activation.
    bool immediateStart; 

  public:

    /*! \short   construct and initialize a AuctionManager object
     */
    AuctionManager( int domain, string fdname, string fvname, bool immediateStart);

    //! destroy a AuctionManager object
    ~AuctionManager(); 
	
	bool getImmediateStart();
	
	Auction * getAuction(string sname, string rname);
	
    //! parse XML auctions from file 
    auctioningObjectDB_t *parseAuctions(string fname, ipap_template_container *templates); 

    //! parse XML or Auction API bids from buffer
    auctioningObjectDB_t *parseAuctionsBuffer(char *buf, int len, ipap_template_container *templates);
   
    //! parse auctions from ipap_message 
    auctioningObjectDB_t *parseMessage(ipap_message *messageIn, ipap_template_container *templates);
   
    /*! \short   add a auction description 

        adding new auctions to the Auction system will parse and syntax
        check the given auction specifications, lookup the auction database for
        already installed auctions and store the auction into the database. 
        This function assigns the UID for all auctions given.

        \throws an Error exception if the given auction description is not
        syntactically correct or does not contain the mandatory fields
        or if a auction with the given identification is already present in the AuctionDatabase
    */
    void addAuctioningObjects(auctioningObjectDB_t *auctions, EventScheduler *e); 

    void delAuction(Auction *a, EventScheduler *e); 

	void delAuction(int uid, EventScheduler *e);

	void delAuction(string sname, string rname, EventScheduler *e);

	void delAuctions(string sname, EventScheduler *e); 
		
	void delAuctioningObjects(auctioningObjectDB_t *auctions, EventScheduler *e);
   

    /*! \short   get the ipap_message that contains all the auctions within
     * 			  the container. 
		\arg     auction - container to put in the message.
		
        \throws an Error exception if some field required is missing.
    */	
	ipap_message * get_ipap_message(auctioningObjectDB_t *auctions, 
									ipap_template_container *templates,
									bool useIPV6, string sAddressIPV4, 
									string sAddressIPV6, uint16_t port);
    
    /*! \short This function return the ids resgistered for a ser of auctions
     */
	void getIds(auctioningObjectDB_t *_auctions, auctionSet_t &setParam);
	
    /*! \short This function increment the session's reference number
     */	
	void incrementReferences(auctionSet_t & setParam, string sessionId);

    /*! \short This function decrement the session's reference number
     */	
	void decrementReferences(auctionSet_t & setParam, string sessionId);
	

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


//! overload for <<, so that a AuctionManager object can be thrown into an iostream
ostream& operator<< ( ostream &os, AuctionManager &rm );

} // namespace auction

#endif // _AUCTION_MANAGER_H_
