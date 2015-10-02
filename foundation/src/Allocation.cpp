
/*! \file   Allocation.cpp

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

    $Id: Allocation.cpp 748 2015-08-20 15:04:00Z amarentes $
*/


#include <sstream>

#include "ParserFcts.h"
#include "Allocation.h"
#include "Error.h"
#include "Timeval.h"

using namespace auction;

Allocation::Allocation( string aset, string aname, string bset, string bname, 
						string allset, string allname, fieldList_t &f, 
						allocationIntervalList_t &alloc_inter )
	: uid(0), state(AL_NEW), auctionSet(aset), auctionName(aname), bidSet(bset), 
      bidName(bname), allocationSet(allset), allocationName(allname),
      fields(f), intervals(alloc_inter) 
{ 

}

Allocation::~Allocation() 
{ 
	
} 

string Allocation::getInfo()
{
	std::stringstream output;

	output << "AuctionSet:" << getAuctionSet() 
		   << " AuctionName:" << getAuctionName()
		   << " BidSet:" << getBidSet() 
		   << " BidName:" << getBidName() 
		   << " AllocationSet:" << getBidSet() 
		   << " AllocationName:" << getBidName() << endl;
	
	allocationIntervalListIter_t inter_iter;
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
	}
	
	return output.str();
}

