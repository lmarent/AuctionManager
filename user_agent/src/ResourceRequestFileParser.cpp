
/*!  \file   ResourceRequestFileParser.cpp

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
    parse resource request files
    Code based on Netmate Implementation

    $Id: ResourceRequestFileParser.cpp 748 2015-08-25 21:18:00 amarentes $

*/

#include "ParserFcts.h" 
#include "Constants.h"
#include "ResourceRequestFileParser.h"
#include "Timeval.h"
#include "ConstantsAgent.h"

using namespace std;
using namespace auction;


ResourceRequestFileParser::ResourceRequestFileParser(string filename)
    : XMLParser(RESOURCE_FILE_DTD, filename, "RESOURCE_REQUEST_SET")
{
    log = Logger::getInstance();
    ch = log->createChannel("ResourceRequestFileParser" );
}


ResourceRequestFileParser::ResourceRequestFileParser(char *buf, int len)
    : XMLParser(RESOURCE_FILE_DTD, buf, len, "RESOURCE_REQUEST_SET")
{
    log = Logger::getInstance();
    ch = log->createChannel("ResourceRequestFileParser" );
}


configItem_t 
ResourceRequestFileParser::parsePref(xmlNodePtr cur)
{
    configItem_t item;

    item.name = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"NAME"));
    if (item.name.empty()) {
        throw Error("Resource Request Parser Error: missing name at line %d", XML_GET_LINE(cur));
    }
    item.value = xmlCharToString(xmlNodeListGetString(XMLDoc, cur->xmlChildrenNode, 1));
    if (item.value.empty()) {
        throw Error("Resource Request Parser Error: missing value at line %d", XML_GET_LINE(cur));
    }
    item.type = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"TYPE"));

    // check if item can be parsed
    try {
        ParserFcts::parseItem(item.type, item.value);
    } catch (Error &e) {    
        throw Error("Resource Request Parser Error: parse value error at line %d: %s", XML_GET_LINE(cur), 
                    e.getError().c_str());
    }

    return item;
}

void 
ResourceRequestFileParser::parseFieldValue(string value, field_t *f)
{
    int n;
	
	// Initialize the values for the field.
	for (int i=0 ; i < MAX_FIELD_SET_SIZE; i++)
	{
		FieldValue fielvalue;
		f->value.push_back(fielvalue);
	}
		
    if (value == "*") {
        f->mtype = FT_WILD;
        f->cnt = 1;
    } else if ((n = value.find("-")) > 0) {
        f->mtype = FT_RANGE;
        f->value[0] = FieldValue(f->type, value.substr(0,n));
        f->value[1] = FieldValue(f->type, value.substr(n+1, value.length()-n+1));
        f->cnt = 2;
    } else if ((n = value.find(",")) > 0) {
        int lastn = 0;
        int c = 0;

        n = -1;
        f->mtype = FT_SET;
        while (((n = value.find(",", lastn)) > 0) && (c<(MAX_FIELD_SET_SIZE-1))) {
            f->value[c] = FieldValue(f->type, value.substr(lastn, n-lastn));
            c++;
            lastn = n+1;
        }
        f->value[c] = FieldValue(f->type, value.substr(lastn, n-lastn));
        f->cnt = c+1;
        if ((n > 0) && (f->cnt == MAX_FIELD_SET_SIZE)) {
            throw Error("more than %d field specified in set", MAX_FIELD_SET_SIZE);
        }
    } else {
        f->mtype = FT_EXACT;
        f->value[0] = FieldValue(f->type, value);
        f->cnt = 1;
    }
}

/* functions for accessing the templates */
string 
ResourceRequestFileParser::getMiscVal(miscList_t *_miscList, string name)
{
    miscListIter_t iter;

    iter = _miscList->find(name);
    if (iter != _miscList->end()) {
        return iter->second.value;
    } else {
        return "";
    }
}


/* ------------------------- parseTime ------------------------- */

time_t 
ResourceRequestFileParser::parseTime(string timestr)
{
    struct tm  t;
  
    if (timestr[0] == '+') {
        // relative time in secs to start
        try {
	    struct tm tm;
            int secs = ParserFcts::parseInt(timestr.substr(1,timestr.length()));
            time_t start = time(NULL) + secs;
            return mktime(localtime_r(&start,&tm));
        } catch (Error &e) {
            throw Error("Incorrect relative time value '%s'", timestr.c_str());
        }
    } else {
        // absolute time
        if (timestr.empty() || (strptime(timestr.c_str(), TIME_FORMAT.c_str(), &t) == NULL)) {
            return 0;
        }
    }
    return mktime(&t);
}


time_t 
ResourceRequestFileParser::calculateInterval(time_t start, miscList_t *miscList, 
							resourceReq_interval_t *resInterval)
{

#ifdef DEBUG
    log->dlog(ch, "start calculateInterval: start:%s ", Timeval::toString(start).c_str());
#endif

    unsigned long duration;
    time_t now = time(NULL);
        
    /* time stuff */
    resInterval->start = start;
        
    // stop = 0 indicates infinite running time
    resInterval->stop = 0;
    
    // duration = 0 indicates no duration set
    duration = 0;
		
	string sstart = getMiscVal(miscList, "Start");
	string sstop = getMiscVal(miscList, "Stop");
	string sduration = getMiscVal(miscList, "Duration");		
	string sinterval = getMiscVal(miscList, "Interval");
	string salign = getMiscVal(miscList, "Align");

    if (!sstart.empty() && !sstop.empty() && !sduration.empty()) {
        throw Error(409, "illegal to specify: start+stop+duration time");
    }
	
    if (!sstart.empty()) {
        resInterval->start = parseTime(sstart);
        if(resInterval->start == 0) {
            throw Error(410, "invalid start time %s", sstart.c_str());
        }
        
        if ((resInterval->start) < start){
			throw Error(410, "invalid start time %s, it should be greater than previous interval stop %s", 
					sstart.c_str(), Timeval::toString(start).c_str());
		}
    }

    if (!sstop.empty()) {
        resInterval->stop = parseTime(sstop);
        if(resInterval->stop == 0) {
            throw Error(411, "invalid stop time %s", sstop.c_str());
        }
        if ((resInterval->stop) < start){
			throw Error(410, "invalid stop time %s, it should be greater than previous interval stop %s", 
					sstop.c_str(), Timeval::toString(start).c_str());
		}

    }
	
    if (!sduration.empty()) {
        duration = ParserFcts::parseULong(sduration);
    }

    if ( duration > 0) {
        if (resInterval->stop) {
            // stop + duration specified
            resInterval->start = resInterval->stop - duration;
        } else {
            // stop [+ start] specified
            resInterval->stop = resInterval->start + duration;
        }
    }
	
    // now start has a defined value, while stop may still be zero 
    // indicating an infinite rule
	    
    // do we have a stop time defined that is in the past ?
    if ((resInterval->stop != 0) && (resInterval->stop <= now)) {
        throw Error(300, "resource request running time is already over");
    }
	
    if (resInterval->start < now) {
        // start late resource request immediately
        resInterval->start = now;
    }
		
    // get export module params
        
    int _interval = 0;
		
	if (!sinterval.empty()){
		_interval = ParserFcts::parseInt(sinterval);
    }
    
    int _align = (!salign.empty()) ? 1 : 0;
				
	if (_interval > 0) {

        resInterval->interval = _interval;
        resInterval->align = _align;   
		
    } else {
		resInterval->interval = 0;
        resInterval->align = _align;
    }				

#ifdef DEBUG
    log->dlog(ch, "Ending Calculate Interval: start:%s, stop:%s, interval:%d - Align:%d", 
						Timeval::toString(resInterval->start).c_str(), 
						Timeval::toString(resInterval->stop).c_str(), 
						_interval, _align);
#endif 
	
	return resInterval->stop;
}


void 
ResourceRequestFileParser::parse(fieldDefList_t *fieldDefs, 
									  resourceRequestDB_t *requests,
									  ResourceRequestIdSource *idSource )
{
    xmlNodePtr cur, cur2, cur3;
    string rset;
    cur = xmlDocGetRootElement(XMLDoc);
    time_t start = time(NULL);

    rset = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"ID"));
	// use lower case internally - set id.
	transform(rset.begin(), rset.end(), rset.begin(), ToLower());
    

#ifdef DEBUG
    log->dlog(ch, "Resource Request Set %s", rset.c_str());
#endif

    cur = cur->xmlChildrenNode;

    while (cur != NULL) {
	
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"RESOURCE_REQUEST")) && (cur->ns == ns)) {
            string rname;
            resourceReqIntervalList_t intervals;
            fieldList_t fields;
            
            time_t start = time(NULL);

            rname = xmlCharToString(xmlGetProp(cur, (const xmlChar *)"ID"));
			// use lower case internally
			transform(rname.begin(), rname.end(), rname.begin(), ToLower());

            cur2 = cur->xmlChildrenNode;

            while (cur2 != NULL) {

                // get FIELD
                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"FIELD")) && (cur2->ns == ns)) {
                    field_t f;
                    fieldDefListIter_t iter;

                    f.name = xmlCharToString(xmlGetProp(cur2, (const xmlChar *)"NAME"));
					
                    if (f.name.empty()) {
                        throw Error("Resource Request Parser Error: missing name at line %d", XML_GET_LINE(cur2));
                    }
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
						string fvalue = xmlCharToString(xmlNodeListGetString(XMLDoc, cur2->xmlChildrenNode, 1));
						
						if (fvalue.empty()) {
						     throw Error("Resource Request Parser Error: missing value at line %d", XML_GET_LINE(cur2));
						}
							
						// parse and set value
						try {
							parseFieldValue(fvalue, &f);
						} catch(Error &e) {
							throw Error("Resource Request Parser Error: field value parse error at line %d: %s", 
											XML_GET_LINE(cur2), e.getError().c_str());
						}

						fields.push_back(f);
					} else {
						throw Error("Resource Request Parser Error: no field definition found at line %d: %s", 
										XML_GET_LINE(cur2), f.name.c_str());
					}
				}
				
                // get INTERVAL.
                if ((!xmlStrcmp(cur2->name, (const xmlChar *)"INTERVAL")) && (cur2->ns == ns)) {
                    resourceReq_interval_t interval;
					miscList_t miscList;
					
                    cur3 = cur2->xmlChildrenNode;
					
                    while (cur3 != NULL) {

						// get PREF
						if ((!xmlStrcmp(cur3->name, (const xmlChar *)"PREF")) && (cur3->ns == ns)) {
							// parse
							configItem_t item = parsePref(cur3); 	
							// add
							miscList[item.name] = item;
#ifdef DEBUG
							log->dlog(ch, "C %s = %s", item.name.c_str(), item.value.c_str());
#endif
						}
						
                        // get action specific PREFs
                        cur3 = cur3->next;
                    }
					start = calculateInterval(start, &miscList, &interval);
					intervals.push_back(interval);
                }
                cur2 = cur2->next;
            }


            // add Resource Request
            try {
                
                ResourceRequest *r = new ResourceRequest(rset, rname, fields, intervals);
#ifdef DEBUG
				// debug info
				log->dlog(ch, "parsed resource request %s.%s", rset.c_str(), rname.c_str());
#endif
                
                requests->push_back(r);
                start = time(NULL);
                
            } catch (Error &e) {
                log->elog(ch, e);
                
                throw e;
            }
        }
        
        cur = cur->next;
    }
}
