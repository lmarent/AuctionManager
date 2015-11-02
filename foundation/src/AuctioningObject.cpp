
/*!  \file   AuctioningObject.cpp

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Quality Manager System (NETAUM).

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
    Defines common classes and methods used for auctioning object.
    Code based on Netmate Implementation

    $Id: AuctioningObject.cpp 748 2015-10-30 8:53:00 amarentes $
*/

#include "AuctioningObject.h"

using namespace auction;


AuctioningObject::AuctioningObject(string name): 
uid(0), state(AO_NEW)
{
    log  = Logger::getInstance();
    ch   = log->createChannel( name );
}


AuctioningObject::~AuctioningObject()
{

}

AuctioningObject::AuctioningObject(const AuctioningObject &rhs):
uid(rhs.uid), state(rhs.state)
{

    log  = Logger::getInstance();
    ch   = rhs.ch;
	
}

string AuctioningObject::getInfo(void)
{

	ostringstream s;

	s << "uid:" << uid << "State:";

    switch (getState()) {
    case AO_NEW:
        s << "new";
        break;
    case AO_VALID:
        s << "validated";
        break;
    case AO_SCHEDULED:
        s << "scheduled";
        break;
    case AO_ACTIVE:
        s << "active";
        break;
    case AO_DONE:
        s << "done";
        break;
    case AO_ERROR:
        s << "error";
        break;
    default:
        s << "unknown";
    }
	
	s << endl;
	
	return s.str();
}
