
/*!\file   TemplateIdSource.cpp

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
	manage unique numeric template id space

    $Id: TemplateIdSource.cpp 2015-08-20 16:56:00Z amarentes $

*/

#include "TemplateIdSource.h"


using namespace auction;

TemplateIdSource *TemplateIdSource::s_instance = NULL;

/* -------------------- getInstance -------------------- */

TemplateIdSource *TemplateIdSource::getInstance()
{ 
    if (s_instance == NULL) {
        s_instance = new TemplateIdSource();
    }
    return s_instance;
}


TemplateIdSource::TemplateIdSource()
  : num(255), unique(0)
{

#ifdef ENABLE_THREADS
    if (threaded) {
        mutexInit(&maccess);
    }
#endif

}


TemplateIdSource::~TemplateIdSource()
{
	
#ifdef ENABLE_THREADS
    if (threaded) {
        mutexDestroy(&maccess);
    }
#endif

}


uint16_t TemplateIdSource::newId(void)
{
    uint16_t id;

	try{

#ifdef ENABLE_THREADS			
        AUTOLOCK(threaded, &maccess);
#endif		
		if (freeIds.empty()) {
			if ( num == 0xFFFF){
				throw Error("TemplateIdSource: Maximum number reached");
			}
			else{
				num++;
			}
			
			while (true)
			{
				IdsReservIterator i = find(idReserved.begin(), idReserved.end(), num);
				if (i != idReserved.end()) {
					if ( num == 0xFFFF){ 
						throw Error("TemplateIdSource: Maximum number reached");
					}
					else {
						num++;
					}
				} else {
					return num;
				}
			}
		}

		// else use id from free list
		id = freeIds.front();
		freeIds.pop_front();

		return id;
	}catch (Error &e) {
        throw e;
    }
}


void TemplateIdSource::freeId(uint16_t id)
{
   try{ 

#ifdef ENABLE_THREADS			
        AUTOLOCK(threaded, &maccess);
#endif
	  
	  if (!unique) {
		freeIds.push_back(id);
		num--;
	  }
   } catch (Error &e){
	   throw e;
   }
}


void TemplateIdSource::dump( ostream &os )
{
    os << "TemplateIdSource dump:" << endl
       << "Number of used ids is : " << num - freeIds.size() << endl;
}


ostream& operator<< ( ostream &os, TemplateIdSource &tis )
{
    tis.dump(os);
    return os;
}
