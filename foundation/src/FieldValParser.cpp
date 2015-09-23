
/*!  \file   FieldValParser.cpp

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
    parse field constant files

    $Id: FieldValParser.cpp 748 2015-07-23 14:33:00Z amarentes $
*/


#include "FieldValParser.h"
#include "Constants.h"

using namespace auction;

FieldValParser::FieldValParser(string filename)
    : XMLParser(FIELDVAL_DTD, filename, "FIELDVAL")
{
    log = Logger::getInstance();
    ch = log->createChannel("FieldValParser" );
}


fieldValItem_t FieldValParser::parseDef(xmlNodePtr cur)
{
    fieldValItem_t item;
    string mask;

    // get DEF
    item.name = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"NAME"));
    if (item.name.empty()) {
        throw Error("Field Val Error: missing name at line %d", XML_GET_LINE(cur));
    }
    // use lower case internally
    transform(item.name.begin(), item.name.end(), item.name.begin(), ToLower());

	item.type = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"TYPE"));
    item.svalue = xmlCharToString(xmlNodeListGetString(XMLDoc, cur->xmlChildrenNode, 1));
    if (item.svalue.empty()) {
        throw Error("Field Val Error: missing value at line %d", XML_GET_LINE(cur));
    }
    // use lower case internally
    transform(item.svalue.begin(), item.svalue.end(), item.svalue.begin(), ToLower());
    // do not parse value here

    return item;
}


void FieldValParser::parse(fieldValList_t *list)
{
    xmlNodePtr cur;

    cur = xmlDocGetRootElement(XMLDoc);

    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
	 
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"DEF")) && (cur->ns == ns)) {
            // parse
            fieldValItem_t item = parseDef(cur);
            // add 
            list->insert(make_pair(item.name, item));
#ifdef DEBUG	
            log->dlog(ch, "%s = %s", item.name.c_str(), item.svalue.c_str());
#endif	
        } 

        cur = cur->next;
    }
}
