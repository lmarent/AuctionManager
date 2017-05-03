
/*!  \file   Auction.h

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Auction manager system (NETAUM).

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
    Defines the resource object.
    Code based on Netmate Implementation

    $Id: Resource.h 748 2015-08-04 9:46:00 amarentes $
*/

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "stdincpp.h"
#include "Constants.h"
#include "Logger.h"
#include "Auction.h"

namespace auction
{

typedef map<string, Auction *>          	  auctionList_t;
typedef map<string, Auction *>::iterator      auctionListIter_t;

class Resource : public AuctioningObject
{

  private:
    
    //! list of auctions associated with this resource
    auctionList_t  auctions;
    
    //! Starting datetime when the resource must be activated.
    time_t  start;
    
    //! Starting datetime when the resource must be activated.
    time_t stop;
    	
  public:    
  
	Resource( string sname, string aname );

    Resource(const Resource &rsh );
    
    ~Resource( );
	    
    //! Add the auction from the resource
    void addAuction(Auction *auction);
    
    //! Deletes the auction from the resource
    void deleteAuction(Auction *auction);
    
    //! Verifies whether or not the auction can be added to the resource
    bool verifyAuction(Auction *auction);

    void setStart(time_t _start);
    
    void setStop(time_t _stop);
    
    time_t getStart();
    
    time_t getStop();

	string getInfo();

    //! Get an iterator to the list of auction running for the resource.
    //auctionListIter_t & getAuctions();

    //! dump a Auction object
    void dump( ostream &os );

        
};


//! overload for <<, so that a Auction object can be thrown into an iostream
ostream& operator<< ( ostream &os, Resource &rs );

} // namespace auction


#endif // _RESOURCE_H_
