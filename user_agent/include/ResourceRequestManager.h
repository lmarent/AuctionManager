
/*! \file   ResourceRequestManager.h

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
    Resource request database
    Code based on Netmate Implementation

	$Id: ResourceRequestManager.h 748 2015-08-25 16:07:00Z amarentes $
*/

#ifndef _RESOURCE_REQUEST_MANAGER_H_
#define _RESOURCE_REQUEST_MANAGER_H_


#include "stdincpp.h"
#include "Logger.h"
#include "Error.h"
#include "ResourceRequestFileParser.h"
#include "AuctioningObjectManager.h"
#include "EventSchedulerAgent.h"

namespace auction
{


//! index bids by time
typedef map<time_t, auctioningObjectDB_t>            resourceRequestTimeIndex_t;
typedef map<time_t, auctioningObjectDB_t>::iterator  resourceRequestTimeIndexIter_t;


/*! \short   manage adding/deleting of complete resource Request descriptions
  
  the ResourceRequestManager class allows to add and remove resource requests in the Auction
  core system. Resource Request data are a set of ascii strings that are parsed
  and syntax checked by the Resource Request Manager and then their respective
  settings are used to configure the other AuctionCore components. 
  The resourceRequest will then be stored in the requestDatabase.
*/

class ResourceRequestManager : public AuctioningObjectManager
{

  public:


    /*! \short   construct and initialize a ResourceRequestManager object
        \arg \c fdname  field definition file name
        \arg \c fvname  field value definition name
     */
    ResourceRequestManager(int domain, string fdname, string fvname); //Ok

    //! destroy a ResourceRequestManager object
    ~ResourceRequestManager(); // Ok

     /*! \short   lookup the ResourceRequest info data for a given ResourceRequestId or name

        lookup the database of ResourceRequests for a specific ResourceRequest
        and return a link (pointer) to the ResourceRequest data associated with it
        do not store this pointer, its contents will be destroyed upon ResourceRequest deletion. 
        do not free this pointer as it is a link to the ResourceRequest and not a copy.
    
        \arg \c resourceRequestId - unique ResourceRequest id
    */
    ResourceRequest *getResourceRequest(int uid);

    //! get ResourceRequest rname from ResourceRequest set and name 
    ResourceRequest *getResourceRequest(string sname, string rname); //Ok

	//! parse XML rules from file 
    auctioningObjectDB_t *parseResourceRequests(string fname); // Ok

    //! parse XML or Auction API ResourceRequests from buffer
    auctioningObjectDB_t *parseResourceRequestsBuffer(char *buf, int len, int mapi);
   
    /*! \short   add a filter rule description 

        adding new ResourceRequests to the Agent system will parse and syntax
        check the given ResourceRequest specifications, lookup the ResourceRequest database for
        already installed ResourceRequests and store the ResourceRequest into the database 

        \throws an Error exception if the given ResourceRequest description is not
        syntactically correct or does not contain the mandatory fields
        or if a ResourceRequest with the given identification is already 
        present in the ResourceRequestDatabase
    */
    void addAuctioningObjects(auctioningObjectDB_t *requests, EventScheduler *e);  


    /*! \short   delete a ResourceRequest description 

        deletion of a ResourceRequest will parse and syntax check the
        identification string, test the presence of the given ResourceRequest
        in the ResourceRequestDatabase, remove the ResourceRequest 
        from the ResourceRequestDatabase

        \throws an Error exception if the given ResourceRequest identification is not
        syntactically correct or does not contain the mandatory fields  or 
        if a ResourceRequest with the given identification is currently not present 
        in the ResourceRequestDatabase
    */
    void delResourceRequest(ResourceRequest *r, EventScheduler *e);
    void delResourceRequest(int uid, EventScheduler *e); //ok
    void delResourceRequest(string rname, string sname, EventScheduler *e); // ok
    void delResourceRequests(string sname, EventScheduler *e);
    void delAuctioningObjects(auctioningObjectDB_t *requests, EventScheduler *e); //Ok
   
    /*! \short   get information from the ResourceRequest manager

        these functions can be used to get information for a single ResourceRequest,
        or a set of ResourceRequests or all ResourceRequests
    */
    string getInfo(void);
    string getInfo(int uid);
    string getInfo(string sname, string rname);
    string getInfo(string sname);


    /*! \short   get the ipap_message that contains the request 
		\arg     request - request to put in the message.
		
        \throws an Error exception if some field required is missing.
    */	
	ipap_message * get_ipap_message(ResourceRequest *request, time_t start,
									string resourceId, bool useIPV6, 
									string sAddressIPV4, string sAddressIPV6, uint16_t port);

    //! dump a ResourceRequestManager object
    void dump( ostream &os );
    
};


//! overload for <<, so that a ResourceRequestManager object can be thrown into an iostream
ostream& operator<< ( ostream &os, ResourceRequestManager &rm );

}; // namespace auction

#endif // _RESOURCE_REQUEST_MANAGER_H_
