
/*!\file   IdSource.cpp

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

    $Id: IdSource.cpp 2015-07-23 13:59:00Z amarentes $

*/

#include "IdSource.h"

using namespace auction;

IdSource::IdSource(int _unique)
  : num(-1), unique(_unique)
{

}


IdSource::~IdSource()
{
    // nothing to do
}


unsigned short IdSource::newId(void)
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


void IdSource::freeId(unsigned short id)
{
  if (!unique) {
    freeIds.push_back(id);
    num--;
  }
}


void IdSource::dump( ostream &os )
{
    os << "IdSource dump:" << endl
       << "Number of used ids is : " << num - freeIds.size() << endl;
}


ostream& operator<< ( ostream &os, IdSource &rim )
{
    rim.dump(os);
    return os;
}
