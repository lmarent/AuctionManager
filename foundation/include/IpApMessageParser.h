
/*!  \file   MAPIIpApMessageParser.h 

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
    common methods for parsers from a API ipap_message

    $Id: MAPIIpApMessageParser.h 748 2015-07-23 17:30:00Z amarentes $
*/

#ifndef _IPAP_MESSAGE_PARSER_H_
#define _IPAP_MESSAGE_PARSER_H_

#include "stdincpp.h"
#include "FieldValParser.h"
#include "ConfigParser.h"
#include "Constants.h"
#include "IpAp_message.h"
#include "AuctioningObject.h"

namespace auction
{

class IpApMessageParser
{
	
	
	public:
		
		//! This field help to define uniquelly an agent with the auction. 
		int domain;
		
		IpApMessageParser(int domain): domain(domain){}
		
		~IpApMessageParser(){}

		//! parse a field value
		static void parseFieldValue(fieldValList_t *fieldVals, string value, field_t *f);

		//! Find a field by eno and ftype within the list of fields.
		static fieldDefItem_t findField(fieldDefList_t *fieldDefs, int eno, int ftype);

		//! Find a field by name within the list of fields.
		static fieldDefItem_t findField(fieldDefList_t *fieldDefs, string fname);

		//! lookup field value
		static string lookup(fieldValList_t *fieldVals, string fvalue, field_t *f);

		//! get a value by name from the misc rule attributes
		static string getMiscVal(miscList_t *miscList, string name);
		
		inline int getDomain(){ return domain; }
	
		static bool isFieldIncluded(auction::fieldList_t *fields, string name);

	
	protected:

		/*! parse an object name (auction or bid).
		  \short   parse identifier format 'setname.objectname'

			recognizes dot (.) in object identifier and saves setname and 
			objectname to the new object
		*/	
		void parseName(string id, string &set, string &name);

		//! parse the type of bidding object
		ipap_object_type_t parseType(string stype);
		
		//! parse the template type
		ipap_templ_type_t parseTemplateType(ipap_object_type_t objectType, string sTemplateType);

		//! Read a template type from a message.
		ipap_template * readTemplate(ipap_message * message, ipap_templ_type_t type);

		dataRecordList_t readDataRecords(ipap_message * message, uint16_t templId);

};

}

#endif // _IPAP_MESSAGE_PARSER_H_
