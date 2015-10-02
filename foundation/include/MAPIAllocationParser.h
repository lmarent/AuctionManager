
/*!  \file   MAPIAllocationParser.h 

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
    parser an allocation from a ipap_message

    $Id: MAPIAllocationParser.h 748 2015-09-30 7:48:00Z amarentes $
*/

#ifndef _MAPI_ALLOCATION_PARSER_H_
#define _MAPI_ALLOCATION_PARSER_H_


#include "stdincpp.h"
#include "IpAp_message.h"
#include "AllocationIdSource.h"
#include "Allocation.h"
#include "MAPIIpApMessageParser.h"

namespace auction
{

//! parser for API text Bid syntax

class MAPIAllocationParser : public MAPIIpApMessageParser
{

  private:

    Logger *log;
    int ch;

	//! Add fields into message's template 
	uint16_t addFieldsRecordTemplate(fieldDefList_t *fieldDefs, 
									 Allocation *allocationPtr, 
									 ipap_message *mes);
									 
	//! Add required field for the allocation's option template 									 
	uint16_t addFieldsOptionTemplate(fieldDefList_t *fieldDefs, 
									 Allocation *allocationPtr, 
									 ipap_message *mes);

	//! Add the field of all auction relationship into option message's template
	void addRecord(string auctionId, string bidId, string allocationId,
				   fieldList_t * fields, fieldDefList_t *fieldDefs,
				   uint16_t templateId, ipap_message *mes );
									  
	void addOptionRecord(string auctionId, string bidId, string allocationId,
						 string recordId, alloc_interval_t allocInt, 
						 uint16_t templateId, ipap_message *mes );

	auction::fieldList_t readAllocationData( ipap_template *templ, fieldDefList_t *fieldDefs,
								    fieldValList_t *fieldVals, ipap_data_record &record, 
								    string &auctionSet, string &auctionName, 
								    string &bidSet, string &bidName, 
								    string &allocationSet, string &allocationName );

	alloc_interval_t readOptionData(ipap_template *templ, 
									fieldDefList_t *fieldDefs,
									ipap_data_record &record);
		   				  
	ipap_message * get_ipap_message(Allocation *bidPtr, 
									fieldDefList_t *fieldDefs);
	
  public:

    MAPIAllocationParser();

    ~MAPIAllocationParser() {}

    //! parse given bids and add parsed bids to bids
    void parse(fieldDefList_t *filters, 
			   fieldValList_t *filterVals,
			   ipap_message *message,
			   allocationDB_t *allocations,
			   AllocationIdSource *idSource,
			   ipap_message *messageOut );
					   
	//! get the ipap_message that represents the set of bids.
	vector<ipap_message *> get_ipap_messages(fieldDefList_t *fieldDefs, 
											 allocationDB_t *allocations);

};

}; // namespace auction


#endif // _MAPI_ALLOCATION_PARSER_H_
