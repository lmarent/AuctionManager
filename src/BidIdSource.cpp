
/*!\file   BidIdSource.cpp

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
	manage unique numeric bid id space - bid idis the flowid.

    $Id: BidIdSource.cpp 2015-07-23 13:59:00Z amarentes $

*/

#include "BidIdSource.h"



BidIdSource::BidIdSource(int _unique)
  : num(0), unique(_unique)
{
	idReserved.push_back(0); 		// Reserve for qdisc
	idReserved.push_back(1);  		// Reserve for root class
	idReserved.push_back(65535);	// Reserve for default class
}


BidIdSource::~BidIdSource()
{
    // nothing to do
}


unsigned short BidIdSource::newId(void)
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


void BidIdSource::freeId(unsigned short id)
{
  if (!unique) {
    freeIds.push_back(id);
    num--;
  }
}


void BidIdSource::dump( ostream &os )
{
    os << "BidIdSource dump:" << endl
       << "Number of used ids is : " << num - freeIds.size() << endl;
}


ostream& operator<< ( ostream &os, BidIdSource &rim )
{
    rim.dump(os);
    return os;
}
