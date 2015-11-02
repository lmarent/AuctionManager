
/*!  \file   MAPIBiddingObjectParser.h 

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
    parser for API text BiddingObject syntax

    $Id: MAPIBiddingObjectParser.h 748 2015-07-23 17:30:00Z amarentes $
*/

#ifndef _MAPI_BIDDING_OBJECT_PARSER_H_
#define _MAPI_BIDDING_OBJECT_PARSER_H_


#include "stdincpp.h"
#include "Logger.h"
#include "BiddingObject.h"
#include "IpAp_message.h"
#include "AuctionFileParser.h"
#include "BiddingObjectFileParser.h"
#include "IpApMessageParser.h"
#include "anslp_ipap_message_splitter.h"

namespace auction
{

//! parser for API text BiddingObject syntax

class MAPIBiddingObjectParser : public IpApMessageParser, public anslp::msg::anslp_ipap_message_splitter
{

  private:

    Logger *log;
    int ch;

	//! Add the field of a data record
	void addDataRecord(fieldDefList_t *fieldDefs, BiddingObject * biddingObjectPtr, string recordId, 
					   fieldList_t &fieldList, uint16_t templateId, ipap_message *mes );	

	//! Add the field of an option data record
	void addOptionRecord(fieldDefList_t *fieldDefs, BiddingObject * biddingObjectPtr, string recordId, 
						 time_t start, time_t stop, fieldList_t &fieldList, uint16_t templateId, 
						 ipap_message *mes );	


	auction::fieldList_t readRecord( ipap_template *templ, fieldDefList_t *fieldDefs,
							    	 fieldValList_t *fieldVals, ipap_data_record &record, 
									 string &auctionSet, string &auctionName,
									 string &biddingObjectSet, string &biddingObjectName, 
									 string &stype, string &status, string &recordId );

	void verifyInsertTemplates(ipap_template *templData, ipap_template *templOption, 
							   ipap_template_container *templatesOut);
	
	bool 
	isTemplateSubtype(ipap_templ_subtype_t sybtype, ipap_templ_type_t type);
	
	ipap_template * findTemplate(ipap_template_container *templatesOut, uint16_t templId);
		   				  
	void get_ipap_message( fieldDefList_t *fieldDefs, BiddingObject *biddingObjectPtr, 
						   Auction *auctionPtr, ipap_template_container *templates, 
						   ipap_message *message);

	void parseAuctionKey( fieldDefList_t *fields, fieldValList_t *fieldVals,
						  const anslp::msg::xml_object_key &key,
						  biddingObjectDB_t *biddingObjects, 
						  ipap_template_container *templates );
	
  public:

    MAPIBiddingObjectParser(int domain);

    ~MAPIBiddingObjectParser() {}

    //! parse given biddingObjects and add parsed biddingObjects to biddingObjects
    void parse( fieldDefList_t *fieldDefs, fieldValList_t *fieldVals,
			    ipap_message *message, biddingObjectDB_t *biddingObjects, 
			    ipap_template_container *templates );


	//! get the ipap_message that represents the set of auctions.
	ipap_message * get_ipap_message(fieldDefList_t *fieldDefs, 
									biddingObjectDB_t *biddingObjects, auctionDB_t *auctions, 
									ipap_template_container *templates );

};

}; // namespace auction


#endif // _MAPI_BIDDING_OBJECT_PARSER_H_
