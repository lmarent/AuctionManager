
/*! \file   ResourceRequest.h

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
    ResourceRequest in the system - All concrete requests for a resource 
    * 	in the system.

    $Id: ResourceRequest.h 748 2015-08-25 15:46:00Z amarentes $
*/

#ifndef _RESOURCE_REQUEST_H_
#define _RESOURCE_REQUEST_H_

#include "stdincpp.h"
#include "Logger.h"
#include "ConfigParser.h"
#include "ProcModuleInterface.h"
#include "AuctionFileParser.h"

namespace auction
{

//! resource request's states during lifecycle
typedef enum
{
    RR_NEW = 0,
    RR_VALID,
    RR_ACTIVE,
    RR_DONE,
    RR_ERROR
} resourceRequestState_t;

//! execution interval definition
typedef struct 
{
    //! resource request interval start
    time_t start;
    //! resource request interval stop
    time_t stop;
    
    //! execution interval
    unsigned long interval;
    //! align yes/no
    int align;
    
} resourceReq_interval_t;



//! execution list intervals.
typedef std::list<resourceReq_interval_t>            	  resourceReqIntervalList_t;
typedef std::list<resourceReq_interval_t>::iterator  	  resourceReqIntervalListIter_t;
typedef std::list<resourceReq_interval_t>::const_iterator resourceReqIntervalListConstIter_t;


class ResourceRequest
{

public:
	
    /*! \short   This function creates a new object instance. 
        \returns a new object instance.
    */
	ResourceRequest( string rset, string rname, fieldList_t &f, resourceReqIntervalList_t &resReqInter );

	~ResourceRequest();

    int getUId() 
    { 
        return uid;
    }
    
    void setUId(int nuid)
    {
        uid = nuid;
    }

    void setState(resourceRequestState_t s) 
    { 
        state = s;
    }

    resourceRequestState_t getState()
    {
        return state;
    }

    string getResourceRequestSet()
    {
        return set;
    }

    void setResourceRequestSet(string rset)
	{
		set = rset;
	}	

   
    string getResourceRequestName()
    {
        return name;
    }

    void setResourceRequestName(string _name)
	{
		name = _name;
	}	

	string getInfo();
		

    /*! \short   get names and values of the allocation's results
        \returns a pointer (link) to a list that contains the results elements for this allocation
    */
    fieldList_t *getFields();
        
    /*! \short   get intervals setup for the allocation
        \returns a pointer (link) to a list that contains the intervals configured for this allocation
    */
    resourceReqIntervalList_t * getIntervals();
    
    resourceReq_interval_t getIntervalByStart(time_t start);

    resourceReq_interval_t getIntervalByEnd(time_t stop);
    
    auctionDB_t * askForAuctions(time_t start);
			
protected:
	
    //! unique bidID of this Rule instance (has to be provided)
    int uid;
   
	//! state of this rule
    resourceRequestState_t state;

	//! resource request set associated
	string set;  

	//! resource request name associated
	string name; 

	//! Resource request fields, two typical fields are quantity and price.
	fieldList_t fields; 

	//! Resource request intervals assigned.
	resourceReqIntervalList_t intervals;

};

}; // namespace auction

#endif // _RESOURCE_REQUEST_H_
