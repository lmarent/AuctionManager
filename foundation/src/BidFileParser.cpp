
/*!  \file   BidFileParser.cpp

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
    parse bid files
    Code based on Netmate Implementation

    $Id: BidFileParser.cpp 748 2015-07-23 17:00:00 amarentes $

*/

#include "ParserFcts.h" 
#include "BidFileParser.h"
#include "Constants.h"
#include "Timeval.h"

using namespace auction;

BidFileParser::BidFileParser(string filename)
    : XMLParser(BIDFILE_DTD, filename, "AGENT"), IpApMessageParser()
{
    log = Logger::getInstance();
    ch = log->createChannel("BidFileParser" );

#ifdef DEBUG
    log->dlog(ch, "BidFileParser Constructor with filename");
#endif

}


BidFileParser::BidFileParser(char *buf, int len)
    : XMLParser(BIDFILE_DTD, buf, len, "AGENT"), IpApMessageParser()
{
    log = Logger::getInstance();
    ch = log->createChannel("BidFileParser" );

#ifdef DEBUG
    log->dlog(ch, "BidFileParser Constructor with buffer");
#endif

}


configItem_t BidFileParser::parsePref(xmlNodePtr cur)
{
    configItem_t item;

    item.name = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"NAME"));
    if (item.name.empty()) {
        throw Error("Bid Parser Error: missing name at line %d", XML_GET_LINE(cur));
    }
    item.value = xmlCharToString(xmlNodeListGetString(XMLDoc, cur->xmlChildrenNode, 1));
    if (item.value.empty()) {
        throw Error("Bid Parser Error: missing value at line %d", XML_GET_LINE(cur));
    }
    item.type = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"TYPE"));

    // check if item can be parsed
    try {
        ParserFcts::parseItem(item.type, item.value);
    } catch (Error &e) {    
        throw Error("Bid Parser Error: parse value error at line %d: %s", XML_GET_LINE(cur), 
                    e.getError().c_str());
    }

    return item;
}


void BidFileParser::parse(fieldDefList_t *fieldDefs, 
						  fieldValList_t *fieldVals, 
						  bidDB_t *bids,
						  BidIdSource *idSource )
{
    xmlNodePtr cur, cur2, cur3;
    string sname, aset, aname, bset, bname;
    cur = xmlDocGetRootElement(XMLDoc);
    time_t now = time(NULL);    

#ifdef DEBUG
    log->dlog(ch, "Starting parse");
#endif


    sname = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"ID"));
	// use lower case internally
	transform(sname.begin(), sname.end(), sname.begin(), ToLower());
    

#ifdef DEBUG
    log->dlog(ch, "agent %s", sname.c_str());
#endif

    cur = cur->xmlChildrenNode;

    while (cur != NULL) {
	
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"BID")) && (cur->ns == ns)) {
            string bname;
            elementList_t elements;
            optionList_t options;

            aname = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"AUCTION_ID"));
			// use lower case internally
			transform(aname.begin(), aname.end(), aname.begin(), ToLower());

			// divide the auction name in set and name
			parseName(aname, aset, aname);

            bname = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"BID_ID"));
			// use lower case internally
			transform(bname.begin(), bname.end(), bname.begin(), ToLower());
			
			// divide the bid name in set and name
			parseName(bname, bset, bname);

            cur2 = cur->xmlChildrenNode;

            while (cur2 != NULL) {

                // get ELEMENT
                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"ELEMENT")) && (cur2->ns == ns)) {
					fieldList_t elemFields;
                    string elemtName = xmlCharToString(xmlGetProp(cur2, (const xmlChar *)"ID"));

                    if (elemtName.empty()) {
                        throw Error("Bid Parser Error: missing name at line %d", XML_GET_LINE(cur2));
                    }
                    // use lower case internally
                    transform(elemtName.begin(), elemtName.end(), elemtName.begin(), 
                              ToLower());

                    cur3 = cur2->xmlChildrenNode;
					
					// TODO: Finish of putting in the correct end array.
                    while (cur3 != NULL) {
						field_t f;
						fieldDefListIter_t iter;
						
                        // get action specific PREFs
                        if ((!xmlStrcmp(cur3->name, (const xmlChar *)"FIELD")) && (cur3->ns == ns)) {
						
							f.name = xmlCharToString(xmlGetProp(cur3, (const xmlChar *)"NAME"));
							
							// use lower case internally
							transform(f.name.begin(), f.name.end(), f.name.begin(), 
										  ToLower());							
										  
							// lookup in field definitions list
							iter = fieldDefs->find(f.name);
							if (iter != fieldDefs->end()) {
								// set according to definition
								f.len = iter->second.len;
								f.type = iter->second.type;

								// lookup in filter var list
								string fvalue = xmlCharToString(xmlNodeListGetString(XMLDoc, cur3->xmlChildrenNode, 1));
								if (fvalue.empty()) {
									throw Error("Bid Parser Error: missing value at line %d", XML_GET_LINE(cur3));
								}
							
								// parse and set value
								try {
									parseFieldValue(fieldVals, fvalue, &f);
									
									
								} catch(Error &e) {
									throw Error("Bid Parser Error: field value parse error at line %d: %s", 
												XML_GET_LINE(cur3), e.getError().c_str());
								}

								elemFields.push_back(f);
							} else {
									throw Error("Bid Parser Error: no field definition found at line %d: %s", 
												XML_GET_LINE(cur3), f.name.c_str());
							}
                        }
                        cur3 = cur3->next;
                    }
					
					elements[elemtName] = elemFields;
                }

                // get OPTION
                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"OPTION")) && (cur2->ns == ns)) {
					fieldList_t optionFields;
                    string optionName = xmlCharToString(xmlGetProp(cur2, (const xmlChar *)"ID"));

                    if (optionName.empty()) {
                        throw Error("Bid Parser Error: missing name at line %d", XML_GET_LINE(cur2));
                    }
                    // use lower case internally
                    transform(optionName.begin(), optionName.end(), optionName.begin(), 
                              ToLower());

                    cur3 = cur2->xmlChildrenNode;
					
					// TODO: Finish of putting in the correct end array.
                    while (cur3 != NULL) {
						field_t f;
						fieldDefListIter_t iter;
						
                        // get action specific PREFs
                        if ((!xmlStrcmp(cur3->name, (const xmlChar *)"PREF")) && (cur3->ns == ns)) {
						
							f.name = xmlCharToString(xmlGetProp(cur3, (const xmlChar *)"NAME"));
							
							// use lower case internally
							transform(f.name.begin(), f.name.end(), f.name.begin(), 
										  ToLower());							
										  
							// lookup in field definitions list
							iter = fieldDefs->find(f.name);
							if (iter != fieldDefs->end()) {
								// set according to definition
								f.len = iter->second.len;
								f.type = iter->second.type;

								// lookup in filter var list
								string fvalue = xmlCharToString(xmlNodeListGetString(XMLDoc, cur3->xmlChildrenNode, 1));
								if (fvalue.empty()) {
									throw Error("Bid Parser Error: missing value at line %d", XML_GET_LINE(cur3));
								}
							
								// parse and set value
								try {
									parseFieldValue(fieldVals, fvalue, &f);
								} catch(Error &e) {
									throw Error("Bid Parser Error: field value parse error at line %d: %s", 
												XML_GET_LINE(cur3), e.getError().c_str());
								}
#ifdef DEBUG
								log->dlog(ch, "field info: %s", (f.getInfo()).c_str());
#endif								
								optionFields.push_back(f);
							} else {
									throw Error("Bid Parser Error: no field definition found at line %d: %s", 
												XML_GET_LINE(cur3), f.name.c_str());
							}
                        }
                        cur3 = cur3->next;
                    }
					
					options.push_back(pair<string,fieldList_t>(optionName, optionFields));
                }

				cur2 = cur2->next;
            }

            // add bid
            try {
                
                Bid *b = new Bid(aset, aname, bset, bname, elements, options);
#ifdef DEBUG
				// debug info
				log->dlog(ch, "parsed bid %s.%s", bset.c_str(), bname.c_str());
#endif
                bids->push_back(b);
            } catch (Error &e) {
                log->elog(ch, e);
                
                throw e;
            }
        }
        
        cur = cur->next;
    }
    
}
