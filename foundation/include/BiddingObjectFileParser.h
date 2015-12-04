
/*!  \file   BiddingObjectFileParser.h

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

    $Id: BiddingObjectFileParser.h 748 2015-07-23 16:09:00 amarentes $
*/

#ifndef _BIDDING_OBJECT_FILEPARSER_H_
#define _BIDDING_OBJECT_FILEPARSER_H_


#include "stdincpp.h"
#include "libxml/parser.h"
#include "Logger.h"
#include "XMLParser.h"
#include "ProcModuleInterface.h"
#include "FieldDefParser.h"
#include "FieldValParser.h"
#include "BiddingObject.h"
#include "IpApMessageParser.h"

namespace auction
{

class BiddingObjectFileParser : public XMLParser, public IpApMessageParser
{
  private:

    Logger *log;
    int ch;

    //! parse a config item
    configItem_t parsePref(xmlNodePtr cur);
    		  
  public:

    BiddingObjectFileParser( int domain, string fname );

    BiddingObjectFileParser( int domain, char *buf, int len );

    virtual ~BiddingObjectFileParser() {}

    //! parse rules and add parsed rules to the list of rules
    virtual void parse(fieldDefList_t *fields, fieldValList_t *fieldVals, biddingObjectDB_t *bids );
};

} // namespace auction

#endif // _BIDDING_OBJECT_FILEPARSER_H_
