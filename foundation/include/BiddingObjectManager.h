
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
#include "BiddingObject.h"
#include "BiddingObjectFileParser.h"
#include "MAPIBiddingObjectParser.h"
#include "EventScheduler.h"
#include "FieldDefManager.h"

namespace auction
{

// default flow idle timeout
const time_t FLOW_IDLE_TIMEOUT = 30;


// RuleDB definition is currently in RuleFileParser.h

// index by set id and name
typedef map<string, int>            				  biddingObjectIndex_t;
typedef map<string, int>::iterator  				  biddingObjectIndexIter_t;
typedef map<string, biddingObjectIndex_t>             biddingObjectSetIndex_t;
typedef map<string, biddingObjectIndex_t>::iterator   biddingObjectSetIndexIter_t;

//! list of done biddingObjects
typedef list<BiddingObject*>            biddingObjectDone_t;
typedef list<BiddingObject*>::iterator  biddingObjectDoneIter_t;

//! index biddingObjects by time
typedef map<time_t, biddingObjectDB_t>            	biddingObjectTimeIndex_t;
typedef map<time_t, biddingObjectDB_t>::iterator  	biddingObjectTimeIndexIter_t;

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

class BiddingObjectManager : public FieldDefManager
{
  private:

    Logger *log;
    int ch; //!< logging channel number used by objects of this class

    //!< number of bidding objects in the database
    int biddingObjects;

    //! index to biddingObjects via setID and name
    biddingObjectSetIndex_t biddingObjectSetIndex;

	//! index biddingObjects via AuctionSetId and Auction name.
	auctionSetBidIndex_t bidAuctionSetIndex;

    //! stores all biddingObjects indexed by setID, bidID
    biddingObjectDB_t  biddingObjectDB;

    //! list with biddingObjects done
    biddingObjectDone_t biddingObjectDone;

    //! pool of unique biddingObject  ids
    IdSource idSource;

	//! This field identifies uniquely the agent.
	int domain; 
	
    /* \short add the biddingObject  name to the list of finished biddingObjects

       \arg \c bidname - name of the finished biddingObject  (source.name)
    */
    void storeBiddingObjectAsDone(BiddingObject  *r);

  public:

    /*! \short   construct and initialize a BiddingObjectManager object
        \arg \c fdname  field definition file name
        \arg \c fvname  field value definition name
     */
    BiddingObjectManager(int domain, string fdname, string fvname); //Ok

    //! destroy a BiddingObjectManager object
    ~BiddingObjectManager(); // Ok

     /*! \short   lookup the biddingObject  info data for a given BidId or name

        lookup the database of biddingObjects for a specific biddingObject 
        and return a link (pointer) to the biddingObject  data associated with it
        do not store this pointer, its contents will be destroyed upon biddingObject  deletion. 
        do not free this pointer as it is a link to the biddingObject  and not a copy.
    
        \arg \c bidId - unique biddingObject  id
    */
    BiddingObject  *getBiddingObject(int uid);

    //! get biddingObject  rname from bidset sname 
    BiddingObject  *getBiddingObject(string sname, string rname); //Ok

    //! get all biddingObjects in bidset with name sname 
    biddingObjectIndex_t *getBiddingObjects(string sname);

	//! get biddingObjects by auction set and auction name
	vector<int> getBiddingObjects(string aset, string aname);

    //! get all biddingObjects
    biddingObjectDB_t getBiddingObjects();

    //! parse XML biddingObjects from file 
    biddingObjectDB_t *parseBiddingObjects(string fname); // Ok

    //! parse XML or Auction API biddingObjects from buffer
    biddingObjectDB_t *parseBiddingObjectsBuffer(char *buf, int len);

    //! parse biddingObjects from ipap_message 
    biddingObjectDB_t *parseMessage(ipap_message *messageIn, ipap_template_container *templates);

   
    /*! \short   add a biddingObject  list

        adding new biddingObjects to the Auction system will parse and syntax
        check the given biddingObject  specifications, lookup the biddingObject  database for
        already installed biddingObjects and store the biddingObject  into the database 

        \throws an Error exception if the given biddingObject  description is not
        syntactically correct or does not contain the mandatory fields
        or if a biddingObject  with the given identification is already present in the bidDatabase
    */
    void addBiddingObjects(biddingObjectDB_t *biddingObjects, EventScheduler *e);  

    //! add a single biddingObject 
    void addBiddingObject(BiddingObject  *b); //ok

    //! activate/execute biddingObjects
    void activateBiddingObjects(biddingObjectDB_t *biddingObjects, EventScheduler *e);

    /*! \short   delete a biddingObject  description 

        deletion of a biddingObject  will parse and syntax check the
        identification string, test the presence of the given biddingObject 
        in the bidDatabase, remove the biddingObject  from the bidDatabase

        \throws an Error exception if the given biddingObject  identification is not
        syntactically correct or does not contain the mandatory fields  or 
        if a biddingObject  with the given identification is currently not present 
        in the bidDatabase
    */
    void delBiddingObject(int uid, EventScheduler *e); //ok
    void delBiddingObject(string rname, string sname, EventScheduler *e); // ok
    void delBiddingObjects(string sname, EventScheduler *e);
    void delBiddingObject(BiddingObject  *r, EventScheduler *e);
    void delBiddingObjects(biddingObjectDB_t *biddingObjects, EventScheduler *e); //Ok
   
    /*! \short   get information from the biddingObject  manager

        these functions can be used to get information for a single biddingObject ,
        or a set of biddingObjects or all biddingObjects
    */
    string getInfo(void);
    
    inline string getInfo(int uid){ return getBiddingObject(uid)->getInfo(); }
    
    string getInfo(string sname, string rname);
    string getInfo(string sname);

    //! dump a BiddingObjectManager object
    void dump( ostream &os );

    //! Return the number of biddingObjects in the container.
    inline int getNumBiddingObjects(){ return biddingObjects; }

	//! Return the domain
	inline int getDomain(){ return domain; }

};


//! overload for <<, so that a BiddingObjectManager object can be thrown into an iostream
ostream& operator<< ( ostream &os, BiddingObjectManager &rm );

}; // namespace auction

#endif // _BIDMANAGER_H_
