
/*!  \file   MAPIAuctionParser.h 

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
    parser for API text auction syntax

    $Id: MAPIAuctionParser.h 748 2015-07-23 17:30:00Z amarentes $
*/

#ifndef _MAPI_AUCTION_PARSER_H_
#define _MAPI_AUCTION_PARSER_H_


#include "stdincpp.h"
#include "AuctionFileParser.h"

//! parser for API text Auction syntax

class MAPIAuctionParser
{

  private:

    Logger *log;
    int ch;
    char *buf;
    int len;
    string fileName;

  public:

    MAPIAuctionParser(string fname);

    MAPIAuctionParser(char *b, int l);

    virtual ~MAPIAuctionParser() {}

    //! parse given bids and add parsed auction to auctions
    virtual void parse(auctionDB_t *auctions,
					   AuctionIdSource *idSource );

};


#endif // _MAPI_AUCTION_PARSER_H_
