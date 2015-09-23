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
    
    parser for API message auction syntax
    $Id: MAPIAuctionParser.h 748 2015-07-23 17:30:00Z amarentes $
*/

#ifndef _MAPI_AUCTION_PARSER_H_
#define _MAPI_AUCTION_PARSER_H_


#include "stdincpp.h"
#include "AuctionFileParser.h"

namespace auction
{

//! parser for API message Auction syntax
class MAPIAuctionParser
{

  private:

    Logger *log;
    int ch;
    ipap_message message;

	//! Read a template type from a message.
	ipap_template * readTemplate(ipap_message * message, ipap_templ_type_t type);
	
	//! Read datarecords associated with a template id.
	vector<ipap_data_record> readDataRecords(ipap_message * message, uint16_t templId);
	
	//! Read the auction data.
	miscList_t readAuctionData(ipap_template *templ, 
							   fieldDefList_t *fieldDefs,
							   ipap_data_record &record,
							   string &auctionName);

    /*! \short   parse identifier format 'sourcename.rulename'

        recognizes dor (.) in task identifier and saves sourcename and 
        rulename to the new malloced strings source and rname
    */
    void parseAuctionName(string aName, string &resource, string &id);
    
    //! Find a field by eno and ftype within the list of fields.
    fieldDefItem_t findField(fieldDefList_t *fieldDefs, int eno, int ftype);

	//! Find a field by name within the list of fields.
	fieldDefItem_t findField(fieldDefList_t *fieldDefs, string name);

	//! Read the action data associated with the auction.
	configItemList_t readMiscAuctionData(ipap_template *templ, 
										 fieldDefList_t *fieldDefs,
										 ipap_data_record &record,
										 string &actionName);

	void getTemplateFields(ipap_template *templ, 
						   fieldDefList_t *fieldDefs,
						   auctionTemplateFieldList_t *templList);
	
	ipap_message * get_ipap_message(Auction *auctionPtr, fieldDefList_t *fieldDefs);
	
  public:

    //! Constructor for MAPIAuctionParser. 
    //! It takes as a parameter a message object and then it is built
    //! the auction.  
    MAPIAuctionParser();

    //! Destructor for MAPIAuctionParser. 
    virtual ~MAPIAuctionParser() {}

    //! parse given bids and add parsed auction to auctions
    virtual void parse( fieldDefList_t *fieldDefs,
						ipap_message *message,
						auctionDB_t *auctions,
					    AuctionIdSource *idSource,
					    ipap_message *messageOut );

	//! get the ipap_message that represents the set of auctions.
	vector<ipap_message *> get_ipap_messages(fieldDefList_t *fieldDefs, 
											auctionDB_t *auctions);

};

}; // namespace auction

#endif // _MAPI_AUCTION_PARSER_H_
