
/*!  \file   MAPIBidParser.h 

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
    parser for API text bid syntax

    $Id: MAPIBidParser.h 748 2015-07-23 17:30:00Z amarentes $
*/

#ifndef _MAPI_BID_PARSER_H_
#define _MAPI_BID_PARSER_H_


#include "stdincpp.h"
#include "BidFileParser.h"

//! parser for API text Bid syntax

class MAPIBidParser
{

  private:

    Logger *log;
    int ch;
    char *buf;
    int len;
    string fileName;

    //! FIXME document!
    void parseFieldValue(fieldValList_t *fieldVals, string value, field_t *f);
    
    string lookup(fieldValList_t *fieldVals, string fvalue, field_t *f);

	//! Calculates intervals associated to auction.
	void calculateIntervals(time_t now, bid_auction_t *auction);

    //! parse time string
    time_t parseTime(string timestr);

    //! get a value by name from the misc rule attriutes
    string getMiscVal(miscList_t *miscList, string name);    


  public:

    MAPIBidParser(string fname);

    MAPIBidParser(char *b, int l);

    virtual ~MAPIBidParser() {}

    //! parse given bids and add parsed bids to bids
    virtual void parse(fieldDefList_t *filters, 
					   fieldValList_t *filterVals, 
					   bidDB_t *bids,
					   BidIdSource *idSource );

};


#endif // _MAPI_BID_PARSER_H_
