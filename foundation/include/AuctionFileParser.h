
/*!  \file   AuctionFileParser.h

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
    parse Auction files. These files contain the auction to be executed.
    Code based on Netmate Implementation

    $Id: AuctionFileParser.h 748 2015-08-04 9:29:00 amarentes $
*/

#ifndef _AUCTIONFILEPARSER_H_
#define _AUCTIONFILEPARSER_H_


#include "stdincpp.h"
#include "libxml/parser.h"
#include "Logger.h"
#include "XMLParser.h"
#include "Auction.h"
#include "ConfigParser.h"
#include "AuctionIdSource.h"
#include "IpAp_message.h"

namespace auction
{

//! auction list
typedef vector<Auction*>            auctionDB_t;
typedef vector<Auction*>::iterator  auctionDBIter_t;

class AuctionFileParser : public XMLParser
{
  private:

    Logger *log;
    int ch;

    //! parse a config item
    configItem_t parsePref(xmlNodePtr cur);    

	//! parse field for templates
	auctionTemplateField_t parseField(xmlNodePtr cur, 
									  fieldDefList_t *fieldDefs,
									  ipap_field_container &g_ipap_fields);


  public:

    AuctionFileParser( string fname );

    AuctionFileParser( char *buf, int len );

    virtual ~AuctionFileParser() {}

    //! parse an auction from the data contained in a xml file. 
    //! put new templates in message.
    virtual void parse( fieldDefList_t *fieldDefs, 
						auctionDB_t *bids,
					    AuctionIdSource *idSource,
					    ipap_template_container *templates );
};

}; // namespace auction

#endif // _AUCTIONFILEPARSER_H_
