
/*!  \file   BidFileParser.h

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
    parse bids files
    Code based on Netmate Implementation

    $Id: BidFileParser.h 748 2015-07-23 16:09:00 amarentes $
*/

#ifndef _BIDFILEPARSER_H_
#define _BIDFILEPARSER_H_


#include "stdincpp.h"
#include "libxml/parser.h"
#include "Logger.h"
#include "XMLParser.h"
#include "Bid.h"
#include "ConfigParser.h"
#include "FieldDefParser.h"
#include "FieldValParser.h"
#include "BidIdSource.h"


//! Bid list
typedef vector<Bid*>            bidDB_t;
typedef vector<Bid*>::iterator  bidDBIter_t;

class BidFileParser : public XMLParser
{
  private:

    Logger *log;
    int ch;

    //! parse a config item
    configItem_t parsePref(xmlNodePtr cur);

    //! lookup field value
    string lookup(fieldValList_t *fieldVals, string fvalue, field_t *f);

	//! parse a field value
	void parseFieldValue(fieldValList_t *fieldVals, string value, field_t *f);   
		
	//! Calculates intervals associated to auction.
	void calculateIntervals(time_t now, bid_auction_t *auction);

    //! parse time string
    time_t parseTime(string timestr);

    //! get a value by name from the misc rule attriutes
    string getMiscVal(miscList_t *miscList, string name);    

  public:

    BidFileParser( string fname );

    BidFileParser( char *buf, int len );

    virtual ~BidFileParser() {}

    //! parse rules and add parsed rules to the list of rules
    virtual void parse(fieldDefList_t *fields, 
					   fieldValList_t *fieldVals, 
					   bidDB_t *bids,
					   BidIdSource *idSource );
};

#endif // _RULEFILEPARSER_H_
