
/*!  \file   FieldDefParser.cc

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

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
    parse field definition files

    $Id: FieldDefParser.cpp 748 2015-07-23 17:00:00 amarentes $
*/

#include "ParserFcts.h"
#include "Constants.h"
#include "FieldDefParser.h"

using namespace auction;

FieldDefParser::FieldDefParser(string filename)
    : XMLParser(FIELDDEF_DTD, filename, "FIELDDEF")
{
    log = Logger::getInstance();
    ch = log->createChannel("FieldDefParser" );
}


fieldDefItem_t FieldDefParser::parseDef(xmlNodePtr cur)
{
    fieldDefItem_t item;
    string mask;
    // get DEF
    item.name = xmlCharToString(xmlNodeListGetString(XMLDoc, cur->xmlChildrenNode, 1));
    if (item.name.empty()) {
        throw Error("Field Def Error: missing name at line %d", XML_GET_LINE(cur));
    }
    // use lower case internally
    transform(item.name.begin(), item.name.end(), item.name.begin(), ToLower());
   
    item.type = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"TYPE"));
    item.len = FieldValue::getTypeLength(item.type);
    
    try {
		item.eno = ParserFcts::parseInt(
			xmlCharToString(xmlGetProp(cur, (const xmlChar *)"ENO")), 0);

		item.ftype = ParserFcts::parseInt(
			xmlCharToString(xmlGetProp(cur, (const xmlChar *)"FTYPE")), 0);		
		
        if ((item.type == "Binary") || (item.type == "String")) {
            item.len = ParserFcts::parseULong(xmlCharToString(xmlGetProp(cur, (const xmlChar *)"LENGTH")),0,
                                              MAX_FIELD_LEN);
        }
    } catch (Error &e) {
        throw Error("Field Def Error: type parse error at line %d: %s",  
                    XML_GET_LINE(cur), e.getError().c_str());
    }
        
    return item;
}


void FieldDefParser::parse(fieldDefList_t *list)
{
    xmlNodePtr cur/*, cur2*/;

    cur = xmlDocGetRootElement(XMLDoc);
		
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
	
        if (((!xmlStrcmp(cur->name, (const xmlChar *)"DEF")) ) && (cur->ns == ns)) {
	 
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"DEF")) && (cur->ns == ns)) {
                
                // parse
                fieldDefItem_t item = parseDef(cur);
                
                // add 
                list->insert(make_pair(item.name, item));
#ifdef DEBUG		
                log->dlog(ch, "%s = |%d", item.name.c_str(), item.len);
#endif
            }
            
        }
        cur = cur->next;
    }

}
