
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
#include "AgentSession.h"
#include "AuctioningObject.h"
#include <uuid/uuid.h>

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
    AuctioningObjectState_t state;
    
    //! resource request interval start
    time_t start;
    //! resource request interval stop
    time_t stop;
    
    //! Session Id to fulfill this interval.
    string sessionId;
    
    //! request process that are fulfilling this resource request interval.
    set<int> resourceProcesses;
    
    //! execution interval
    unsigned long interval;
    //! align yes/no
    int align;
    
} resourceReq_interval_t;



//! execution list intervals.
typedef std::vector<resourceReq_interval_t>            	  resourceReqIntervalList_t;
typedef std::vector<resourceReq_interval_t>::iterator  	  resourceReqIntervalListIter_t;
typedef std::vector<resourceReq_interval_t>::const_iterator resourceReqIntervalListConstIter_t;


class ResourceRequest : public AuctioningObject
{

public:
	
    /*! \short   This function creates a new object instance. 
        \returns a new object instance.
    */
	ResourceRequest( string rset, string rname, fieldList_t &f, resourceReqIntervalList_t &resReqInter );

	~ResourceRequest();

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
	
	string getIpApId(int domain);
	
	field_t *getField(string name);

	string getInfo();
		

    /*! \short   get names and values of the allocation's results
        \returns a pointer (link) to a list that contains the results elements for this allocation
    */
    fieldList_t *getFields();
        
    /*! \short   get intervals setup for the allocation
        \returns a pointer (link) to a list that contains the intervals configured for this allocation
    */
    resourceReqIntervalList_t * getIntervals();
    
    resourceReqIntervalListIter_t  getIntervalByStart(time_t start);

    resourceReqIntervalListIter_t  getIntervalByEnd(time_t stop);
    
    /*! \short   Assign the session created for the interval.
     * 
    */
    void assignSession(time_t start, time_t stop, string sessionId);
								  
	/*! \short This function return the session associated with the interval start
	 * 	 \returns the session id in string.
	 */
	string getSession(time_t start);
			
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
