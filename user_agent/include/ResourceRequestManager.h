
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
#include "EventSchedulerAgent.h"
#include "ResourceRequestFileParser.h"
#include "MAPIResourceRequestParser.h"


// index by set id and name
typedef map<string, int>            					resourceRequestIndex_t;
typedef map<string, int>::iterator  					resourceRequestIndexIter_t;
typedef map<string, resourceRequestIndex_t>             resourceRequestSetIndex_t;
typedef map<string, resourceRequestIndex_t>::iterator   resourceRequestSetIndexIter_t;

//! list of done resource request
typedef list<ResourceRequest*>            resourceRequestDone_t;
typedef list<ResourceRequest*>::iterator  resourceRequestDoneIter_t;

//! index bids by time
typedef map<time_t, resourceRequestDB_t>            resourceRequestTimeIndex_t;
typedef map<time_t, resourceRequestDB_t>::iterator  resourceRequestTimeIndexIter_t;


/*! \short   manage adding/deleting of complete resource Request descriptions
  
  the BidManager class allows to add and remove bids in the Auction
  core system. Resource Request data are a set of ascii strings that are parsed
  and syntax checked by the BidManager and then their respective
  settings are used to configure the other AuctionCore components. 
  The resourceRequest will then be stored in the bidDatabase inside the BidManager
*/

class ResourceRequestManager
{
  private:

    Logger *log;
    int ch; //!< logging channel number used by objects of this class

    //!< number of resource requests in the database
    int resourceRequests;

    //! index to bids via setID and name
    resourceRequestSetIndex_t resourceRequestSetIndex;

    //! stores all resource requests indexed by setID, resource requestID
    resourceRequestDB_t  resourceRequestDB;

    //! list with resource requests done
    resourceRequestDone_t resourceRequestDone;

	//! filter definitions
    fieldDefList_t fieldDefs;

    // name of filter def and filter vals files
    string fieldDefFileName;

    //! load filter definitions
    void loadFieldDefs(string fname);

    // pool of unique resourceRequest ids
    ResourceRequestIdSource idSource;

    /* \short add the resourceRequest name to the list of finished resourceRequests

       \arg \c resourceRequestname - name of the finished resourceRequest (source.name)
    */
    void storeResourceRequestAsDone(ResourceRequest *r);

  public:

    int getNumResourceRequests() 
    { 
        return resourceRequests; 
    }

    string getInfo(int uid)
    {
        return getInfo(getResourceRequest(uid)); 
    }

    /*! \short   construct and initialize a ResourceRequestManager object
        \arg \c fdname  field definition file name
        \arg \c fvname  field value definition name
     */
    ResourceRequestManager(string fdname); //Ok

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

    //! get all ResourceRequests in the ResourceRequest set with name sname 
    resourceRequestIndex_t *getResourceRequests(string sname);

    //! get all rules
    resourceRequestDB_t getResourceRequests();

    //! parse XML rules from file 
    resourceRequestDB_t *parseResourceRequests(string fname); // Ok

    //! parse XML or Auction API ResourceRequests from buffer
    resourceRequestDB_t *parseResourceRequestsBuffer(char *buf, int len, int mapi);
   
    /*! \short   add a filter rule description 

        adding new ResourceRequests to the Agent system will parse and syntax
        check the given ResourceRequest specifications, lookup the ResourceRequest database for
        already installed ResourceRequests and store the ResourceRequest into the database 

        \throws an Error exception if the given ResourceRequest description is not
        syntactically correct or does not contain the mandatory fields
        or if a ResourceRequest with the given identification is already 
        present in the ResourceRequestDatabase
    */
    void addResourceRequests(resourceRequestDB_t *requests, EventScheduler *e);  

    //! add a single ResourceRequest
    void addResourceRequest(ResourceRequest *b); //ok

    //! activate/execute ResourceRequests
    void activateResourceRequests(resourceRequestDB_t *requests, EventScheduler *e);

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
    void delResourceRequest(int uid, EventScheduler *e); //ok
    void delResourceRequest(string rname, string sname, EventScheduler *e); // ok
    void delResourceRequests(string sname, EventScheduler *e);
    void delResourceRequest(ResourceRequest *r, EventScheduler *e);
    void delResourceRequests(resourceRequestDB_t *requests, EventScheduler *e); //Ok
   
    /*! \short   get information from the ResourceRequest manager

        these functions can be used to get information for a single ResourceRequest,
        or a set of ResourceRequests or all ResourceRequests
    */
    string getInfo(void);
    string getInfo(ResourceRequest *r);
    string getInfo(string sname, string rname);
    string getInfo(string sname);

    //! dump a ResourceRequestManager object
    void dump( ostream &os );

    fieldDefList_t *getFieldDef() //ok
    {
		return &fieldDefs;
	}
    
};


//! overload for <<, so that a ResourceRequestManager object can be thrown into an iostream
ostream& operator<< ( ostream &os, ResourceRequestManager &rm );


#endif // _RESOURCE_REQUEST_MANAGER_H_
