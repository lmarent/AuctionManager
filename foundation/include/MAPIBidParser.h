
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
#include "Logger.h"
#include "Bid.h"
#include "IpAp_message.h"
#include "BidFileParser.h"
#include "MAPIIpApMessageParser.h"

namespace auction
{

//! parser for API text Bid syntax

class MAPIBidParser : public MAPIIpApMessageParser
{

  private:

    Logger *log;
    int ch;

	//! Add the field of all elements into message's template 
	uint16_t addElementFieldsTemplate(fieldDefList_t *fieldDefs, 
									  Bid *bidPtr, ipap_message *mes);

	//! Add required field for the bid's option template 									 
	uint16_t addFieldsOptionTemplate(fieldDefList_t *fieldDefs, 
									 Bid *bidPtr, 
									 ipap_message *mes);

	//! Add the field of all auction relationship into option message's template
	void addOptionRecord(string bidId, int recordId, bid_auction_t bAuct, 
						 uint16_t bidTemplateId, ipap_message *mes );
									  
	
	//! Add all fields of an element to a new record data.
	void addElementRecord(string bidId, 
						  string elementId, 
						  fieldList_t *elemFields, 
						  fieldDefList_t *fieldDefs, 
						  uint16_t bidTemplateId, 
						  ipap_message *mes );
	

	auction::fieldList_t readBidData( ipap_template *templ, fieldDefList_t *fieldDefs,
						   fieldValList_t *fieldVals, ipap_data_record &record, 
						   string &bidSet, string &bidName, string &elementName );

	bid_auction_t readBidOptionData(ipap_template *templ, 
					  			    fieldDefList_t *fieldDefs,
									ipap_data_record &record);
		   				  
	ipap_message * get_ipap_message(Bid *bidPtr, fieldDefList_t *fieldDefs);
	
  public:

    MAPIBidParser();

    ~MAPIBidParser() {}

    //! parse given bids and add parsed bids to bids
    void parse(fieldDefList_t *filters, 
					   fieldValList_t *filterVals,
					   ipap_message *message,
					   bidDB_t *bids,
					   BidIdSource *idSource,
					   ipap_message *messageOut );
					   
	//! get the ipap_message that represents the set of bids.
	vector<ipap_message *> get_ipap_messages(fieldDefList_t *fieldDefs, 
											 bidDB_t *auctions);

};

}; // namespace auction


#endif // _MAPI_BID_PARSER_H_
