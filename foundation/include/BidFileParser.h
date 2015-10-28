
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
#include "ProcModuleInterface.h"
#include "FieldDefParser.h"
#include "FieldValParser.h"
#include "BidIdSource.h"
#include "Bid.h"
#include "IpApMessageParser.h"

namespace auction
{

class BidFileParser : public XMLParser, public IpApMessageParser
{
  private:

    Logger *log;
    int ch;

    //! parse a config item
    configItem_t parsePref(xmlNodePtr cur);
		  
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

}; // namespace auction

#endif // _RULEFILEPARSER_H_
