
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

#include "ParserFcts.h"
#include "Constants.h"
#include "AuctionFileParser.h"

using namespace auction;

AuctionFileParser::AuctionFileParser(int domain, string filename)
    : XMLParser(AUCTIONFILE_DTD, filename, "AUCTIONSET") , IpApMessageParser(domain)
{
    log = Logger::getInstance();
    ch = log->createChannel("AuctionFileParser");
}


AuctionFileParser::AuctionFileParser(int domain, char *buf, int len)
    : XMLParser(AUCTIONFILE_DTD, buf, len, "AUCTIONSET"), IpApMessageParser(domain)
{
    log = Logger::getInstance();
    ch = log->createChannel("AuctionFileParser" );
}


configItem_t AuctionFileParser::parsePref(xmlNodePtr cur)
{
    configItem_t item;

	item.name = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"NAME"));
	// use lower case internally
	transform(item.name.begin(), item.name.end(), item.name.begin(), ToLower());
    
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


auctionTemplateField_t AuctionFileParser::parseField(xmlNodePtr cur, 
										   fieldDefList_t *fieldDefs,
										   ipap_field_container &g_ipap_fields)
{

#ifdef DEBUG
    log->dlog(ch, "Starting parseField");
#endif

    xmlNodePtr cur2;
    
    
    // check if item can be parsed
    try {

		auctionTemplateField_t fieldTempl;
		string sname, sObjectType, sTemplateType;

		sname = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"NAME"));
		// use lower case internally
		transform(sname.begin(), sname.end(), sname.begin(), ToLower());
		
		// lookup in field definitions list
		fieldDefListIter_t iter;
		iter = fieldDefs->find(sname);
		if (iter != fieldDefs->end()) {
			// set according to definition
			ipap_field field = g_ipap_fields.get_field(iter->second.eno, iter->second.ftype);

			fieldTempl.field = iter->second;
			fieldTempl.length = field.get_field_type().length;
			
		}
		else{
			throw Error("Auction Parser Error: field name %s at \
							line %d not valid", sname.c_str(), XML_GET_LINE(cur));
        }

		// load the bidding object's templates to which the field belong to
        cur2 = cur->xmlChildrenNode;
		set< pair<ipap_object_type_t,ipap_templ_type_t> > _fieldBelonging;
		
        while (cur2 != NULL) {
           // get action specific PREFs
           if ((!xmlStrcmp(cur2->name, (const xmlChar *)"TEMPLATE_FIELD")) && (cur2->ns == ns)) {
			   
			   // Parse object type
			   sObjectType = xmlCharToString(xmlGetProp(cur2, (const xmlChar *)"OBJECT_TYPE"));
			   
			   transform(sObjectType.begin(), sObjectType.end(), sObjectType.begin(), ToLower());
			   ipap_object_type_t objectType = parseType(sObjectType);
			   
			   // Parse template type
			   sTemplateType = xmlCharToString(xmlGetProp(cur2, (const xmlChar *)"TEMPLATE_TYPE"));
			   transform(sTemplateType.begin(), sTemplateType.end(), sTemplateType.begin(), ToLower());
			   ipap_templ_type_t templateType = parseTemplateType(objectType, sTemplateType);
			
			   std::pair<ipap_object_type_t,ipap_templ_type_t> tmp = 
					std::pair<ipap_object_type_t,ipap_templ_type_t>(objectType, templateType);
			   _fieldBelonging.insert(tmp);
           }
           cur2 = cur2->next;
        }
    
		// Copy data into returning variable.
		fieldTempl.fieldBelongTo = _fieldBelonging;
		
		
#ifdef DEBUG
		log->dlog(ch, "Ending parseField");
#endif
		
		return fieldTempl;

    } catch(ipap_bad_argument &e) {
		throw Error("Auction Parser Error: invalid ipap_field \
						at line %d", XML_GET_LINE(cur));
	} catch (Error &e) {    
        throw Error("Auction Parser Error: parse value error at \
					line %d: %s", XML_GET_LINE(cur), e.getError().c_str());
   }

}


void AuctionFileParser::parse( fieldDefList_t *fieldDefs, 
							   auctioningObjectDB_t *auctions,
							   ipap_template_container *templates )
{

    xmlNodePtr cur, cur2, cur3;
    string gset;
    actionList_t globalActionList;
    miscList_t globalMiscList;
    time_t now = time(NULL);
    string defaultActGbl;

	// load field definitions for ipap_messages.
	ipap_field_container g_ipap_fields;	
    g_ipap_fields.initialize_forward();
    g_ipap_fields.initialize_reverse();


    cur = xmlDocGetRootElement(XMLDoc);

    gset = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"ID"));
    // use lower case internally
    transform(gset.begin(), gset.end(), gset.begin(), ToLower());
	

#ifdef DEBUG
    log->dlog(ch, "Global Auction set %s", gset.c_str());
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
					// use lower case internally
					transform(a.name.begin(), a.name.end(), a.name.begin(), ToLower());

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
            string sid, sname, rname, resourceSet, resourceId;
            actionList_t actions = globalActionList;
            miscList_t miscs = globalMiscList;
			auctionTemplateFieldList_t templFields;

            sid = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"ID"));
            transform(sid.begin(), sid.end(), sid.begin(), ToLower());
            parseName(sid, sname, rname);

			if (sname.empty()){
				try{
					int iset = ParserFcts::parseInt(gset);
				} catch (Error &e) {
					gset= ""; // the number given is not an integer.
				}
				if (gset.empty()){
					ostringstream o;
					o << getDomain();
					sname = o.str();
				} else {
					sname = gset;
				}
			}
			            
            resourceSet = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"RESOURCE_SET"));
            resourceId = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"RESOURCE_ID"));
            transform(resourceSet.begin(), resourceSet.end(), resourceSet.begin(), ToLower());
            transform(resourceId.begin(), resourceId.end(), resourceId.begin(), ToLower());

            cur2 = cur->xmlChildrenNode;

            while (cur2 != NULL) {

                // get rule specific PREFs
                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"PREF")) && (cur2->ns == ns)) {
                    // parse
                    configItem_t item = parsePref(cur2); 	
                    // add
                    miscs[item.name] = item;
                }

                // get FIELD
                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"FIELD")) && (cur2->ns == ns)) {
                    // parse
                    auctionTemplateField_t templField = parseField(cur2, fieldDefs, g_ipap_fields); 	
                    // add
                    templFields[templField.field.name] = templField;
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
				
                Auction *a = new Auction(now, sname, rname, resourceSet, resourceId, action, 
										 miscs, AS_BUILD_TEMPLATE, 
										 templFields, templates );
                auctions->push_back(a);
            } catch (Error &e) {
                log->elog(ch, e);
                
                throw e;
            }
        }
        
        cur = cur->next;
    }

}
