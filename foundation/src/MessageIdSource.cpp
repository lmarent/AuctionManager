
/*!\file   MessageIdSource.cpp

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
	manage unique numeric for Message ids within a session space

    $Id: MessageIdSource.cpp 2015-10-29 11:11:00Z amarentes $

*/

#include "MessageIdSource.h"
#include <openssl/rand.h>
#include <openssl/engine.h>


using namespace auction;

MessageIdSource::MessageIdSource()
{
	// Start from a random number within [0,2 power 32]

	unsigned char  buffer[4];
	
	if (! (RAND_bytes(buffer, sizeof(buffer))) ) {
		throw Error("MessageIdSource: The PRNG is not seeded!");
	}
	
	num = (uint32_t)buffer[0] << 24 |
		  (uint32_t)buffer[1] << 16 |
		  (uint32_t)buffer[2] << 8  |
		  (uint32_t)buffer[3];
		
}


MessageIdSource::~MessageIdSource()
{
    // nothing to do
}


uint32_t MessageIdSource::newId(void)
{
    
    if ( (num + 1 ) == UINT32_MAX ) 
		num = 0;
	else
		num++;

    return num;
}



void MessageIdSource::dump( ostream &os )
{
    os << "MessageIdSource dump:" << endl
       << "Next number is : " << num  << endl;
}


ostream& auction::operator<< ( ostream &os, MessageIdSource &aim )
{
    aim.dump(os);
    return os;
}
