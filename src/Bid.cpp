
/*! \file   Bid.cpp

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
    Bids in the system - All concrete bids inherit from this class.

    $Id: Bid.cpp 748 2015-07-23 15:30:00Z amarentes $
*/

#include "Bid.h"
#include "Error.h"


/* ------------------------- Bid ------------------------- */

Bid::Bid(int _uid,  elementList_t &elements)
  : uid(_uid), state(BS_NEW), elementList(elements)
{
    unsigned long duration;

    log = Logger::getInstance();
    ch = log->createChannel("Bid");

#ifdef DEBUG
    log->dlog(ch, "Bid constructor");
#endif    

}


Bid::~Bid()
{
#ifdef DEBUG
    log->dlog(ch, "Bid destructor");
#endif    

}

/* ------------------------- getActions ------------------------- */

elementList_t *Bid::getElements()
{
    return &elementList;
}
