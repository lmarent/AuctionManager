
/*! \file   Resource.cpp

    Copyright 2014-2016 Universidad de los Andes, Bogotá, Colombia

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
    Auctions in the system.

    $Id: Resource.cpp 748 2016-04-28 15:31:00Z amarentes $
*/

#include "Resource.h"


using namespace auction;

/* ------------------------- Auction ------------------------- */

Resource::Resource( string sname, string aname ) : 
AuctioningObject("RESOURCE", sname, aname)
{


}

Resource::Resource( const Resource &rhs ) : 
AuctioningObject(rhs)
{


}

Resource::~Resource( )
{


}


//! Add the auction from the resource
void Resource::addAuction(Auction *auction)
{

#ifdef DEBUG    
	log->dlog(ch, "starting addAuction");
#endif  
	
	assert(auction != NULL);
	
	string index = auction->getSet() + "." + auction->getName();
	
	auctionListIter_t iter = auctions.find(index);
	
	if ( iter == auctions.end())
	{
		auctions[index] = auction;
	} 
	else {
		throw Error("Auction with name:%s already inserted in the resource %s", 
						index.c_str(), (getSet() + "." + auction->getName()).c_str());
	}
	
#ifdef DEBUG    
	log->log(ch, "ending addAuction");	
#endif  
}
    
//! Deletes the auction from the resource
void Resource::deleteAuction(Auction *auction)
{

#ifdef DEBUG    
	log->log(ch, "starting deleteAuction");
#endif  
	
	assert(auction != NULL);
	
	string index = auction->getSet() + "." + auction->getName();
	
	auctionListIter_t iter = auctions.find(index);
	
	if ( iter != auctions.end()){
		auctions.erase (iter);
	}
	
#ifdef DEBUG    
	log->log(ch, "ending deleteAuction");
#endif  

}
    
//! Verifies whether or not the auction can be added to the resource
bool Resource::verifyAuction(Auction *auction)
{

	bool valRet = true;
	assert(auction != NULL);
	auctionListIter_t iter;
	
	time_t startA = auction->getStart();
	time_t stopA = auction->getStop();
		
	for (iter = auctions.begin(); iter != auctions.end(); iter++ )
	{
		
		time_t start = (iter->second)->getStart();
		time_t stop = (iter->second)->getStop();
		
		if ((startA <= start) && ((start <= stopA) && (stopA <= stop)))
			return false;
		
		if ((start <= startA) && (stopA <= stop))
			return false;

		if ((startA <= stop) && (stop <= stopA))
			return false;
	}
	
	return valRet;
}

void 
Resource::setStart(time_t _start)
{
	start = _start;
}    

void 
Resource::setStop(time_t _stop)
{
	stop = _stop;
}
    
time_t 
Resource::getStart()
{
	return start;
}
    
time_t 
Resource::getStop()
{
	return stop;
}

/* ------------------------- dump ------------------------- */

void Resource::dump( ostream &os )
{
    os << "Resource dump :" << endl;

	os << "name:" << getSet() << "." << getName() << " ";

}

/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, Resource &r )
{
    r.dump(os);
    return os;
}

/* ------------------------- getInfo ------------------------- */

string Resource::getInfo(void)
{
    ostringstream s;

	s << AuctioningObject::getInfo();

    s << getSet() << "." << getName() << " ";

	s << getStart() << " & " << getStop() << " ";

    s << endl;

    return s.str();	
}
