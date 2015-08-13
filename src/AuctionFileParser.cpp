
/*!  \file   AuctionFileParser.cpp

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

    $Id: AuctionFileParser.cpp 748 2015-08-04 9:41:00 amarentes $

*/

#include "AuctionFileParser.h"
#include "ParserFcts.h"
#include "Constants.h"


AuctionFileParser::AuctionFileParser(string filename)
    : XMLParser(AUCTIONFILE_DTD, filename, "AUCTIONSET")
{
    log = Logger::getInstance();
    ch = log->createChannel("AuctionFileParser" );
}


AuctionFileParser::AuctionFileParser(char *buf, int len)
    : XMLParser(AUCTIONFILE_DTD, buf, len, "AUCTIONSET")
{
    log = Logger::getInstance();
    ch = log->createChannel("AuctionFileParser" );
}


configItem_t AuctionFileParser::parsePref(xmlNodePtr cur)
{
    configItem_t item;

    item.name = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"NAME"));
    if (item.name.empty()) {
        throw Error("Auction Parser Error: missing name at line %d", XML_GET_LINE(cur));
    }
    item.value = xmlCharToString(xmlNodeListGetString(XMLDoc, cur->xmlChildrenNode, 1));
    if (item.value.empty()) {
        throw Error("Auction Parser Error: missing value at line %d", XML_GET_LINE(cur));
    }
    item.type = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"TYPE"));

    // check if item can be parsed
    try {
        ParserFcts::parseItem(item.type, item.value);
    } catch (Error &e) {    
        throw Error("Auction Parser Error: parse value error at line %d: %s", XML_GET_LINE(cur), 
                    e.getError().c_str());
    }

    return item;
}

void AuctionFileParser::parse( auctionDB_t *auctions,
							   AuctionIdSource *idSource )
{

    xmlNodePtr cur, cur2, cur3;
    string sname;
    actionList_t globalActionList;
    time_t now = time(NULL);
    string defaultActGbl;

    cur = xmlDocGetRootElement(XMLDoc);

    sname = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"ID"));

#ifdef DEBUG
    log->dlog(ch, "Auction set %s", sname.c_str());
#endif

    cur = cur->xmlChildrenNode;

    while (cur != NULL) {
	
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"GLOBAL")) && (cur->ns == ns)) {
            // parse global settings

            cur2 = cur->xmlChildrenNode;

            while (cur2 != NULL) {
                // get PREF
                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"PREF")) && (cur2->ns == ns)) {
                    // parse
                    configItem_t item = parsePref(cur2); 	
                    // add
                    globalMiscList[item.name] = item;
#ifdef DEBUG
                    log->dlog(ch, "C %s = %s", item.name.c_str(), item.value.c_str());
#endif
                }

                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"ACTION")) && (cur2->ns == ns)) {
                    action_t a;

                    a.name = xmlCharToString(xmlGetProp(cur2, (const xmlChar *)"NAME"));
                    if (a.name.empty()) {
                        throw Error("Auction Parser Error: missing name at line %d", XML_GET_LINE(cur2));
                    }

                    defaultActGbl = xmlCharToString(xmlGetProp(cur2, (const xmlChar *)"DEFAULT"));
                    if (defaultActGbl.empty()) {
                        throw Error("Auction Parser Error: missing name at line %d", XML_GET_LINE(cur2));
                    }

					// check if the default value given can be parsed.
					try {
						a.defaultAct = ParserFcts::parseBool(defaultActGbl);
						
					} catch (Error &e) {    
						throw Error("Auction Parser Error: parse value error at line %d: %s", XML_GET_LINE(cur2), 
									e.getError().c_str());
					}


                    cur3 = cur2->xmlChildrenNode;

                    while (cur3 != NULL) {
                        // get action specific PREFs
                        if ((!xmlStrcmp(cur3->name, (const xmlChar *)"PREF")) && (cur3->ns == ns)) {
                            configItem_t item;
                            // parse
                            item = parsePref(cur3); 	
                            // add
                            a.conf.push_back(item);
                        }

                        cur3 = cur3->next;
                    }

                    globalActionList.push_back(a);
                }
	
                cur2 = cur2->next;
            }
        }
	
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"AUCTION")) && (cur->ns == ns)) {
            string rname;
            actionList_t actions = globalActionList;
            miscList_t miscs = globalMiscList;

            rname = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"ID"));

            cur2 = cur->xmlChildrenNode;

            while (cur2 != NULL) {

                // get rule specific PREFs
                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"PREF")) && (cur2->ns == ns)) {
                    // parse
                    configItem_t item = parsePref(cur2); 	
                    // add
                    miscs[item.name] = item;

                }
       
                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"ACTION")) && (cur2->ns == ns)) {
                    action_t a;
                    string defaultAct;

                    a.name = xmlCharToString(xmlGetProp(cur2, (const xmlChar *)"NAME"));
                    if (a.name.empty()) {
                        throw Error("Auction Parser Error: missing name at line %d", XML_GET_LINE(cur2));
                    }

					a.defaultAct = 1; // Establishes this as the default action.
						
                    cur3 = cur2->xmlChildrenNode;

                    while (cur3 != NULL) {
                        // get action specific PREFs
                        if ((!xmlStrcmp(cur3->name, (const xmlChar *)"PREF")) && (cur3->ns == ns)) {
                            configItem_t item;
                            // parse
                            item = parsePref(cur3); 	
                            // add
                            a.conf.push_back(item);
                        }

                        cur3 = cur3->next;
                    }

					// overide the global action parameter
					for (actionListIter_t i=actions.begin(); i != actions.end(); ++i) {
						(*i).defaultAct = 0; // Establish other actions as non default.
						if (i->name == a.name) {
							actions.erase(i);
						}
					}
                    actions.push_back(a);
                }

                cur2 = cur2->next;
            }

#ifdef DEBUG
            // debug info
            log->dlog(ch, "auction %s.%s", sname.c_str(), rname.c_str());
            for (actionListIter_t i = actions.begin(); i != actions.end(); i++) {
                log->dlog(ch, " A %s", i->name.c_str());
                for (configItemListIter_t j = i->conf.begin(); j != i->conf.end(); j++) {
                    log->dlog(ch, "  C %s = %s", j->name.c_str(), j->value.c_str());
                }
            }
            for (miscListIter_t i = miscs.begin(); i != miscs.end(); i++) {
                log->dlog(ch, " C %s = %s", i->second.name.c_str(), i->second.value.c_str());
            }
#endif

            // add auction
            try {
				action_t action;
				// overide the global action parameter
				for (actionListIter_t i=actions.begin(); i != actions.end(); ++i) {
					if (i->defaultAct == 1) {
						action = *i;
						break;
					}
				}
				
                Auction *a = new Auction(now, sname, rname, action, 
										 miscs );
                auctions->push_back(a);
            } catch (Error &e) {
                log->elog(ch, e);
                
                throw e;
            }
        }
        
        cur = cur->next;
    }

}
