
/*! \file   AuctioningObjectManager.h

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
    Resource db
    Code based on Netmate Implementation

	$Id: auctioningObjectManager.h 748 2016-04-28 16:12:00Z amarentes $
*/

#ifndef _AUCTIONING_OBJECT_MANAGER_H_
#define _AUCTIONING_OBJECT_MANAGER_H_


#include "stdincpp.h"
#include "Logger.h"
#include "Error.h"
#include "IdSource.h"
#include "AuctioningObject.h"
#include "EventScheduler.h"
#include "FieldDefManager.h"

namespace auction
{

// AuctioningObjectDB definition is currently in AuctionFileParser.h

// index by set id and name
typedef map<string, int>            		    	  	auctioningObjectIndex_t;
typedef map<string, int>::iterator  		    	  	auctioningObjectIndexIter_t;
typedef map<string, auctioningObjectIndex_t>            auctioningObjectSetIndex_t;
typedef map<string, auctioningObjectIndex_t>::iterator  auctioningObjectSetIndexIter_t;

//! list of done Auction Object
typedef list<AuctioningObject*>            auctioningObjectDone_t;
typedef list<AuctioningObject*>::iterator  auctioningObjectDoneIter_t;


/*! \short   manage adding/deleting of auctioning object descriptions
  
  the auctioningObjectManager class allows to add and remove Auction Objects 
  in the core system. Auctioning objects data are a set of ascii strings that are parsed
  and syntax checked by the auctioningObjectManager and then their respective
  settings are used to configure the other Core components. 
*/

class AuctioningObjectManager : public FieldDefManager
{
  private:

    //!< number of resources in the database
    int objects;

    //! index to auction objects via setID and name
    auctioningObjectSetIndex_t auctioningObjectSetIndex;

    //! stores all auction objects indexed by setID, bidID
    auctioningObjectDB_t  auctioningObjectDB;

    //! list with auction object done
    auctioningObjectDone_t auctioningObjectDone;
		
	//! This field identifies uniquely the agent.
	int domain; 


  protected:

    Logger *log;
    int ch; //!< logging channel number used by objects of this class


    /*! \short add the auctioning object to the list of finished bids

       \arg \a Auctioning Object
    */
    void storeAuctioningObjectAsDone(AuctioningObject *a);
  
    //! pool of unique ids
    IdSource idSource;


  public:

    /*! \short   construct and initialize a auctioningObjectManager object
     */
    AuctioningObjectManager( int domain, string fdname, string fvname, string channelName);

    //! destroy a auctioningObjectManager object
    virtual ~AuctioningObjectManager(); 

     /*! \short   lookup the auction object info data for a given auctionId or name

        lookup the database of auction objects for a specific auction object
        and return a link (pointer) to the data associated with it
        do not store this pointer, its contents will be destroyed upon auction object deletion. 
        do not free this pointer as it is a link to the auction and not a copy.
    
        \arg \c uId - unique auction object id
    */
    AuctioningObject *getAuctioningObject(int uid);

    //! get auction object rname from auctionset sname 
    AuctioningObject *getAuctioningObject(string sname, string rname); 

	//! get auction object with name name and set sname from the stored mark as done.
	AuctioningObject *getAuctioningObjectDone(string sname, string aoname);

	//! get auction object with id uid from the stored mark as done.
	AuctioningObject *getAuctioningObjectDone(int uid);

    //! get all Auctioning Objects in auctionset with name sname 
    auctioningObjectIndex_t *getAuctioningObjects(string sname);

    //! get all auctioning objects, creates a new vector with 
    //! the same pointers to auction objects, so it does not 
    //! require to free memory. 
    auctioningObjectDB_t getAuctioningObjects();
   
    /*! \short   add a auction description 

        adding new auction objects to the Auction system will parse and syntax
        check the given auction object specifications, lookup the database for
        already installed auction objects and store the auction object into the database. 
        This function assigns the UID for all auction objects given.

        \throws an Error exception if the given auction description is not
        syntactically correct or does not contain the mandatory fields
        or if a auction object with the given identification is already present in the Database
    */
    virtual void addAuctioningObjects(auctioningObjectDB_t *auctioningObjects, EventScheduler *e) = 0; 

    //! add a single auction, the methods assigns the UID for the auction.
    void addAuctioningObject(AuctioningObject *b); 

    //! activate/execute bids
    void activateAuctioningObjects(auctioningObjectDB_t *auctioningObjects);


    /*! \short   delete a Auction Object description 

        deletion of a auction object will parse and syntax check the
        identification string, test the presence of the given auction object
        in the database, remove the auction object from the database

        \throws an Error exception if the given auction object identification is not
        syntactically correct or does not contain the mandatory fields  or 
        if a auction object with the given identification is currently not present 
        in the database.
    */
    void delAuctioningObject(int uid); 
    void delAuctioningObject(string sname, string rname); 
    void delAuctioningObjects(string sname); 
    void delAuctioningObject(AuctioningObject *a); 
    virtual void delAuctioningObjects(auctioningObjectDB_t *auctioningObjects, EventScheduler *e) = 0; 
   
    //! Return the number of auctioning objects in the container.
    inline int getNumAuctioningObjects() { return objects; }

	//! Return the domain
	inline int getDomain(){ return domain; }
    		
};


} // namespace auction

#endif // _AUCTION_OBJECT_MANAGER_H_
