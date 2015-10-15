
/*!  \file   MAPIResourceRequestParser.h 

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
    parser for API ipap_message syntax.

    $Id: MAPIResourceRequestParser.h 748 2015-07-23 17:30:00Z amarentes $
*/

#ifndef _MAPI_RESOURCE_REQUEST_PARSER_H_
#define _MAPI_RESOURCE_REQUEST_PARSER_H_



#include "stdincpp.h"
#include "Logger.h"
#include "ResourceRequest.h"
#include "IpAp_message.h"
#include "MAPIIpApMessageParser.h"

namespace auction
{

//! parser for API ipap_message syntax

class MAPIResourceRequestParser : public MAPIIpApMessageParser
{

  private:

    Logger *log;
    int ch;


	//! Add required field for the bid's option template 									 
	uint16_t addFieldsOptionTemplate(fieldDefList_t *fieldDefs, 
									 ipap_message *mes);

	//! Add the field of all auction relationship into option message's template
	void addOptionRecord(string recordId,
						 string resourceId,
						 resourceReq_interval_t interval, 
						 bool useIPV6, string sAddressIPV4, 
						 string sAddressIPV6, uint16_t port, 
						 uint16_t bidTemplateId, 
						 ipap_message *mes );
									  
		   				  
	
  public:

    MAPIResourceRequestParser();

    ~MAPIResourceRequestParser() {}
					   
	//! get the ipap_message that represents an specifc resource request.
	/*! @param recordId - it identifies the request being processed.
	 * 		  resourceId - It identifies the resource request, if empty
	 * 					   means any resource.
	 */
	ipap_message * get_ipap_message(fieldDefList_t *fieldDefs, 
								    string recordId,
									string resourceId,
								    resourceReq_interval_t interval,
								    bool useIPV6, string sAddressIPV4, 
								    string sAddressIPV6, uint16_t port);

};

} // namespace auction


#endif // _MAPI_RESOURCE_REQUEST_PARSER_H_
