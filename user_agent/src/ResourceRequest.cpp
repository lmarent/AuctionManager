
/*! \file   ResourceRequest.cpp

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
    Allocations in the system - All concrete allocations inherit from this class.

    $Id: ResourceRequest.cpp 748 2015-08-25 15:57:00Z amarentes $
*/


#include "ParserFcts.h"
#include "ResourceRequest.h"
#include "Error.h"
#include "Timeval.h"

using namespace auction;

ResourceRequest::ResourceRequest( string rset, string rname, fieldList_t &f, 
								resourceReqIntervalList_t &resReqInter )
	: AuctioningObject("RESOURCE_REQUEST", rset, rname), fields(f), intervals(resReqInter) 
{ 

}

ResourceRequest::~ResourceRequest() 
{ 
	
} 

string 
ResourceRequest::getIpApId(int domain)
{
	string idResourceRequestS;
	if ((getSet()).empty()){
		ostringstream ssA;
		ssA << domain;
		idResourceRequestS =  ssA.str() + "." + getName();
	} else {
		idResourceRequestS = getSet() + "." + getName();
	}
	
	return idResourceRequestS;
}



string ResourceRequest::getInfo()
{
	std::stringstream output;
	
	output << AuctioningObject::getInfo();
	
	output << " Set:" << getSet() 
		   << " Name:" << getName() << endl;
	
	resourceReqIntervalListIter_t inter_iter;
	int counter = 1;
	for (inter_iter = intervals.begin(); inter_iter != intervals.end(); ++inter_iter)
	{
		output << "Interval: #" << counter <<endl
			   << " start:" << Timeval::toString(inter_iter->start)
			   << " stop:" << Timeval::toString(inter_iter->stop) << endl;
		++counter;
	}
	
	fieldListIter_t field_iter;
	counter = 1;
	for (field_iter = fields.begin(); field_iter != fields.end(); ++field_iter)
	{
		output << "Field: #" << counter <<endl 
			   << field_iter->getInfo() << endl;
		++counter;
	}
	
	return output.str();
}

auction::fieldList_t *ResourceRequest::getFields()
{
	return &fields;
}

resourceReqIntervalList_t * ResourceRequest::getIntervals()
{
	return &intervals;
}

resourceReqIntervalListIter_t
ResourceRequest::getIntervalByStart(time_t start)
{
		
	resourceReqIntervalListIter_t inter_iter;
	for (inter_iter = intervals.begin(); inter_iter != intervals.end(); ++inter_iter)
	{
		if (inter_iter->start == start) {
			return inter_iter;
		}
	}	
	
	return intervals.end();
}


resourceReqIntervalListIter_t 
ResourceRequest::getIntervalByEnd(time_t stop)
{
	
	resourceReqIntervalListIter_t inter_iter;
	for (inter_iter = intervals.begin(); inter_iter != intervals.end(); ++inter_iter)
	{
		if (inter_iter->stop == stop) {
			return inter_iter;
		}
	}	
	
	return intervals.end();
}

field_t *
ResourceRequest::getField(string name)
{
    fieldListIter_t iter;

    for (iter = fields.begin(); iter != fields.end(); ++iter){
		if (name.compare(iter->name) == 0){
			return &(*iter);
		} 
    }
    return NULL;
}

void
ResourceRequest::assignSession( time_t start, time_t stop, string sessionId )
{

	
	// Assign the session for the interval
	resourceReqIntervalListIter_t inter_iter;
	for (inter_iter = intervals.begin(); inter_iter != intervals.end(); ++inter_iter)
	{
		if ((inter_iter->start == start) && (inter_iter->stop == stop)) {
			inter_iter->sessionId = sessionId;
		}
	}	
		
}

/*----------------- getSession --------------------------*/
string ResourceRequest::getSession(time_t start)
{
	string valReturn;
	
	// Get the session from the interval
	resourceReqIntervalListIter_t inter_iter;
	for (inter_iter = intervals.begin(); inter_iter != intervals.end(); ++inter_iter)
	{
		if (inter_iter->start == start) {
			valReturn = inter_iter->sessionId;
			break;
		}
	}	
	
	return valReturn;
}
