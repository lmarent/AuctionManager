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
#include "MAPIIpApMessageParser.h"
#include "anslp_ipap_message_splitter.h"


namespace auction
{

//! parser for API message Auction syntax
class MAPIAuctionParser : public MAPIIpApMessageParser, public anslp::msg::anslp_ipap_message_splitter
{

  private:

    Logger *log;
    int ch;
	
	//! Read the auction data.
	miscList_t readAuctionData(ipap_template *templ, 
							   fieldDefList_t *fieldDefs,
							   ipap_data_record &record,
							   string &auctionName);
    
	//! Read the action data associated with the auction.
	configItemList_t readMiscAuctionData(ipap_template *templ, 
										 fieldDefList_t *fieldDefs,
										 ipap_data_record &record,
										 string &actionName);

	void getTemplateFields(ipap_template *templ, 
						   fieldDefList_t *fieldDefs,
						   auctionTemplateFieldList_t *templList);
		
	void get_ipap_message(fieldDefList_t *fieldDefs, Auction *auctionPtr,
						  ipap_template_container *templates,
						  int domainId, bool useIPV6, string sAddressIPV4, 
						  string sAddressIPV6, uint16_t port, ipap_message *);
	
   /**
    * Verify that templData and templOption are equal to those in the container
    * if there are not inserted, insert them in the container.
    * @param templData		- data template to check
    *        templOption    - option template to check.
    * 		 templatesOut   - Template container where we are going to look for.
    * @throws Error templates are not the same. 
    */
	void verifyInsertTemplates(ipap_template *templData, ipap_template *templOption, 
							   ipap_template_container *templatesOut);
							   
   /**
    * Find a template in the template container given as parameter.
    * @param templData		- if the template id given corresponds to a data template,
    * 						  then this pointer is updated and returned.
    *        templOption    - if the template id given corresponds to a option template,
    * 						  then this pointer is updated and returned.
    * 		 templatesOut   - Template container where we are going to look
    * 		 templId	 	- Id of the template searched.
    * @return pointer to the template object.
    * @throws Error when the template is not found. 
    */
	ipap_template * findTemplate(ipap_template *templData, ipap_template *templOption,
								 ipap_template_container *templatesOut, 
								 uint16_t templId);
	
	void parseAuctionKey( fieldDefList_t *fieldDefs, const anslp::msg::xml_object_key &key, 
						  auctionDB_t *auctions, ipap_template_container *templatesOut );
	
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
					    ipap_template_container *templatesOut );

	//! get the ipap_message that represents the set of auctions.
	ipap_message * get_ipap_message(fieldDefList_t *fieldDefs, 
									auctionDB_t *auctions, 
									ipap_template_container *templates,
									int domainId, bool useIPV6, string sAddressIPV4, 
									string sAddressIPV6, uint16_t port );

};

} // namespace auction

#endif // _MAPI_AUCTION_PARSER_H_
