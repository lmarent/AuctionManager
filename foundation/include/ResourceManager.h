
/*! \file   ResourceManager.h

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

	$Id: ResourceManager.h 748 2015-08-04 13:31:00Z amarentes $
*/

#ifndef _RESOURCE_MANAGER_H_
#define _RESOURCE_MANAGER_H_


#include "stdincpp.h"
#include "Logger.h"
#include "Error.h"
#include "Auction.h"
#include "AuctioningObjectManager.h"
#include "EventScheduler.h"
#include "Resource.h"

namespace auction
{

// index by set id and name
typedef map<string, int>            			 resourceIndex_t;
typedef map<string, int>::iterator  			 resourceIndexIter_t;
typedef map<string, resourceIndex_t>             resourceSetIndex_t;
typedef map<string, resourceIndex_t>::iterator   resourceSetIndexIter_t;

//! list of done resources
typedef list<Resource*>            resourceDone_t;
typedef list<Resource*>::iterator  resourceDoneIter_t;

//! index resources by time
typedef map<time_t, auctioningObjectDB_t>            resourceTimeIndex_t;
typedef map<time_t, auctioningObjectDB_t>::iterator  resourceTimeIndexIter_t;

typedef set<int> 		    resourceSet_t;
typedef set<int>::iterator  resourceSetIter_t;

/*! \short   manage adding/deleting of complete auction descriptions
  
  the ResourceManager class allows to add and remove resources in the Auction
  core system. 
  The Resource will then be stored in the ResourceDatabase inside the ResourceManager
*/

class ResourceManager : public AuctioningObjectManager
{


  public:

    /*! \short   construct and initialize a ResourceManager object
     */
    ResourceManager( int domain, string fdname, string fvname);

    //! destroy a ResourceManager object
    ~ResourceManager(); 
	
	Resource * getResource(string sname, string rname);
	
    /*! \short   add a resource description 

        adding new resources to the Auction system, it will parse and 
        check the syntax of the given resource specification. 
        Lookup the resource database for already installed resources 
        and store the resource into the database. 
        
        This function assigns the UID for all resources given.

        \throws an Error exception if the given resource description is not
        syntactically correct or does not contain the mandatory fields
        or if a resource with the given identification is already 
        present in the AuctionDatabase
    */
    void addAuctioningObjects(auctioningObjectDB_t *resources, EventScheduler *e); 

    void delResource(Resource *a, EventScheduler *e); 

	void delResource(int uid, EventScheduler *e);

	void delResource(string sname, string rname, EventScheduler *e);

	void delResources(string sname, EventScheduler *e); 
		
	void delAuctioningObjects(auctioningObjectDB_t *resources, EventScheduler *e);
   	

    /*! \short   get information from the auction manager

        these functions can be used to get information for a single resource object,
        or a set of resource objects
    */
    string getInfo(int uid);
    string getInfo(string sname, string rname);
    string getInfo(string sname);

	/*! \short   Verify the list of auctions can be included 
	 * 			  in all of the resources
	 */
	bool verifyAuctions(auctioningObjectDB_t *auctions);

    //! dump a ResourceManager object
    void dump( ostream &os );
	
};


//! overload for <<, so that a ResourceManager object can be thrown into an iostream
ostream& operator<< ( ostream &os, ResourceManager &rm );

} // namespace auction

#endif // _RESOURCE_MANAGER_H_
