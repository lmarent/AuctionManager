
/*!\file   AuctionIdSource.cpp

    Copyright 2014-2015 Universidad de los Andes, Bogota, Colombia

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
	manage unique numeric auction id space.

    $Id: AuctionIdSource.cpp 2015-08-04 14:52:00Z amarentes $

*/

#include "AuctionIdSource.h"

using namespace auction;

AuctionIdSource::AuctionIdSource(int _unique)
  : num(-1), unique(_unique)
{

}


AuctionIdSource::~AuctionIdSource()
{
    // nothing to do
}


unsigned short AuctionIdSource::newId(void)
{
    unsigned short id;
    
    if (freeIds.empty()) {
        num++;
        while (true)
        {
			IdsReservIterator i = find(idReserved.begin(), idReserved.end(), num);
			if (i != idReserved.end()) {
				num++;
			} else {
				return num;
			}
		}
    }

    // else use id from free list
    id = freeIds.front();
    freeIds.pop_front();

    return id;
}


void AuctionIdSource::freeId(unsigned short id)
{
  if (!unique) {
    freeIds.push_back(id);
    num--;
  }
}


void AuctionIdSource::dump( ostream &os )
{
    os << "AuctionIdSource dump:" << endl
       << "Number of used ids is : " << num - freeIds.size() << endl;
}


ostream& operator<< ( ostream &os, AuctionIdSource &ais )
{
    ais.dump(os);
    return os;
}
