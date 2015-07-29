
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

#include "BidFileParser.h"
#include "ParserFcts.h"
#include "Constants.h"


BidFileParser::BidFileParser(string filename)
    : XMLParser(BIDFILE_DTD, filename, "BIDSET")
{
    log = Logger::getInstance();
    ch = log->createChannel("BidFileParser" );
}


BidFileParser::BidFileParser(char *buf, int len)
    : XMLParser(BIDFILE_DTD, buf, len, "BIDSET")
{
    log = Logger::getInstance();
    ch = log->createChannel("BidFileParser" );
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
        throw Error("Rule Parser Error: parse value error at line %d: %s", XML_GET_LINE(cur), 
                    e.getError().c_str());
    }

    return item;
}

string 
BidFileParser::lookup(fieldValList_t *fieldVals, string fvalue, field_t *f)
{
    fieldValListIter_t iter2 = fieldVals->find(fvalue);
    if (iter2 != fieldVals->end()) {
        if (iter2->second.type == f->type) {
            // substitute filter value
            fvalue = iter2->second.svalue;
        } else {
            throw Error("element value type mismatch: %s given but %s expected", 
                        iter2->second.type.c_str(), f->type.c_str());
        }
    }

    return fvalue;
}


void 
BidFileParser::parseFieldValue(fieldValList_t *fieldVals, string value, field_t *f)
{
    int n;

    if (value == "*") {
        f->mtype = FT_WILD;
        f->cnt = 1;
    } else if ((n = value.find("-")) > 0) {
        f->mtype = FT_RANGE;
        f->value[0] = FieldValue(f->type, lookup(fieldVals, value.substr(0,n),f));
        f->value[1] = FieldValue(f->type, lookup(fieldVals, value.substr(n+1, value.length()-n+1),f));
        f->cnt = 2;
    } else if ((n = value.find(",")) > 0) {
        int lastn = 0;
        int c = 0;

        n = -1;
        f->mtype = FT_SET;
        while (((n = value.find(",", lastn)) > 0) && (c<(MAX_FIELD_SET_SIZE-1))) {
            f->value[c] = FieldValue(f->type, lookup(fieldVals, value.substr(lastn, n-lastn),f));
            c++;
            lastn = n+1;
        }
        f->value[c] = FieldValue(f->type, lookup(fieldVals, value.substr(lastn, n-lastn),f));
        f->cnt = c+1;
        if ((n > 0) && (f->cnt == MAX_FIELD_SET_SIZE)) {
            throw Error("more than %d field specified in set", MAX_FIELD_SET_SIZE);
        }
    } else {
        f->mtype = FT_EXACT;
        f->value[0] = FieldValue(f->type, lookup(fieldVals, value,f));
        f->cnt = 1;
    }
}


void BidFileParser::parse(fieldDefList_t *fieldDefs, 
						  fieldValList_t *fieldVals, 
						  bidDB_t *bids,
						  BidIdSource *idSource )
{
    xmlNodePtr cur, cur2, cur3;
    string sname;

    cur = xmlDocGetRootElement(XMLDoc);

    sname = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"ID"));

#ifdef DEBUG
    log->dlog(ch, "bidset %s", sname.c_str());
#endif

    cur = cur->xmlChildrenNode;

    while (cur != NULL) {
	
	
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"BID")) && (cur->ns == ns)) {
            string rname;
            elementList_t elements;
            fieldList_t fields;

            rname = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"ID"));

            cur2 = cur->xmlChildrenNode;

            while (cur2 != NULL) {

                // get ELEMENT
                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"ELEMENT")) && (cur2->ns == ns)) {
                    element_t elem;

                    elem.name = xmlCharToString(xmlGetProp(cur2, (const xmlChar *)"NAME"));
                    if (elem.name.empty()) {
                        throw Error("Rule Parser Error: missing name at line %d", XML_GET_LINE(cur2));
                    }
                    // use lower case internally
                    transform(elem.name.begin(), elem.name.end(), elem.name.begin(), 
                              ToLower());

                    cur3 = cur2->xmlChildrenNode;
					
					// TODO: Finish of putting in the correct end array.
                    while (cur3 != NULL) {
						field_t f;
						fieldDefListIter_t iter;
						
                        // get action specific PREFs
                        if ((!xmlStrcmp(cur3->name, (const xmlChar *)"FIELD")) && (cur3->ns == ns)) {
						
							f.name = xmlCharToString(xmlGetProp(cur3, (const xmlChar *)"NAME"));
							
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
								// use lower case internally
								transform(f.name.begin(), f.name.end(), f.name.begin(), 
										  ToLower());
							
								// parse and set value
								try {
									parseFieldValue(fieldVals, fvalue, &f);
								} catch(Error &e) {
									throw Error("Bid Parser Error: field value parse error at line %d: %s", 
												XML_GET_LINE(cur3), e.getError().c_str());
								}

								elem.fields.push_back(f);
							} else {
									throw Error("Bid Parser Error: no field definition found at line %d: %s", 
												XML_GET_LINE(cur3), f.name.c_str());
							}
                        }
                        cur3 = cur3->next;
                    }
					
					elements.push_back(elem);
                }
       
                cur2 = cur2->next;
            }

#ifdef DEBUG
            // debug info
            log->dlog(ch, "bid %s.%s", sname.c_str(), rname.c_str());
            /*
             * TODO AM: Manage more than one value by field 
            for (fieldListIter_t i = fields.begin(); i != fields.end(); i++) {
                switch (i->mtype) {
                case FT_WILD:
                    log->dlog(ch, " F %s&%s = *", i->name.c_str(), i->mask.getString().c_str());
                    break;
                case FT_EXACT:
                    log->dlog(ch, " F %s&%s = %s", i->name.c_str(), i->mask.getString().c_str(), 
                              i->value[0].getString().c_str());
                    break;
                case FT_RANGE:
                    log->dlog(ch, " F %s&%s = %s-%s", i->name.c_str(), i->mask.getString().c_str(), 
                              i->value[0].getString().c_str(), i->value[1].getString().c_str() );
                    break;
                case FT_SET:
                    string vals;
                    for (int j=0; j < i->cnt; j++) {
                        vals += i->value[j].getString();
                        if (j < (i->cnt-1)) {
                            vals += ", ";
                        }
                    }
                    log->dlog(ch, " F %s&%s = %s", i->name.c_str(), i->mask.getString().c_str(), 
                              vals.c_str());
                    break;
                }
            }
			*/
#endif

            // add bid
            try {
				unsigned short uid = idSource->newId();
                Bid *b = new Bid((int) uid, elements);
                bids->push_back(b);
            } catch (Error &e) {
                log->elog(ch, e);
                
                throw e;
            }
        }
        
        cur = cur->next;
    }
}
