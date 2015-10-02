
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
	: uid(0), state(RR_NEW), set(rset), name(rname), fields(f), intervals(resReqInter) 
{ 

}

ResourceRequest::~ResourceRequest() 
{ 
	
} 

string ResourceRequest::getInfo()
{
	std::stringstream output;

	output << "set:" << getResourceRequestSet() 
		   << " Name:" << getResourceRequestName() << endl;
	
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

resourceReq_interval_t ResourceRequest::getIntervalByStart(time_t start)
{
	
	resourceReq_interval_t inter_return;
	inter_return.start = 0;
	inter_return.stop = 0;
	inter_return.interval = 0;
	inter_return.align = 0;
	
	resourceReqIntervalListIter_t inter_iter;
	for (inter_iter = intervals.begin(); inter_iter != intervals.end(); ++inter_iter)
	{
		if (inter_iter->start == start) {
			inter_return = *inter_iter;
		}
	}	
	
	return inter_return;
}


resourceReq_interval_t ResourceRequest::getIntervalByEnd(time_t stop)
{
	
	resourceReq_interval_t inter_return;
	inter_return.start = 0;
	inter_return.stop = 0;
	inter_return.interval = 0;
	inter_return.align = 0;
	
	resourceReqIntervalListIter_t inter_iter;
	for (inter_iter = intervals.begin(); inter_iter != intervals.end(); ++inter_iter)
	{
		if (inter_iter->stop == stop) {
			inter_return = *inter_iter;
		}
	}	
	
	return inter_return;
}
