
/*!  \file   MAPIResourceRequestParser.cpp

    Copyright 2014-2015 Universidad de los Andes, Bogota, Colombia

    This file is part of Network Auction Manager System (NETAuM).

    NETAuM is free software; you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    NETAuM is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Description:
    parser for API text resource request syntax

    $Id: MAPIResourceRequestParser.cpp 2015-08-26 08:10:00 amarentes $
*/

#include "ParserFcts.h"
#include "Constants.h"
#include "MAPIResourceRequestParser.h"
#include "Timeval.h"

using namespace auction;

MAPIResourceRequestParser::MAPIResourceRequestParser(string filename)
    : fileName(filename)
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIResourceRequestParser" );
}


MAPIResourceRequestParser::MAPIResourceRequestParser(char *b, int l)
    : buf(b), len(l)
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIResourceRequestParser" );
}


void MAPIResourceRequestParser::parseFieldValue( string value, field_t *f)
{
    int n;

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
string MAPIResourceRequestParser::getMiscVal(miscList_t *_miscList, string name)
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
time_t MAPIResourceRequestParser::parseTime(string timestr)
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


void MAPIResourceRequestParser::calculateInterval(time_t now, miscList_t *miscList, 
							resourceReq_interval_t *resInterval)
{

    unsigned long duration;
        
    /* time stuff */
    resInterval->start = now;
        
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
    }

    if (!sstop.empty()) {
        resInterval->stop = parseTime(sstop);
        if(resInterval->stop == 0) {
            throw Error(411, "invalid stop time %s", sstop.c_str());
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
			
#ifdef DEBUG
    log->dlog(ch, "Interval: %d - Align:%d", _interval, _align);
#endif    
		
    }				

}


void MAPIResourceRequestParser::parse(fieldDefList_t *fieldDefs, 
									  resourceRequestDB_t *requests,
									  ResourceRequestIdSource *idSource )
{
    string rset, rname;

    istringstream in(buf);
    string args[128];
    int argc = 0;
    int ind = 0;
    string tmp;
    int n = 0, n2 = 0;
    string line;
    time_t now = time(NULL);

    // each line contains 1 resource request
    while (getline(in, line)) {    
        // tokenize rule string

#ifdef DEBUG
            log->dlog(ch, "Line given: %s", line.c_str());
#endif
		
        resourceReqIntervalList_t intervals;
        fieldList_t fields;
		
        // skip initial white ws
        n = line.find_first_not_of(" \t", 0);
        while ((n2 = line.find_first_of(" \t", n)) > 0) {
            if (argc >= (int) (sizeof(args)-1)) {
                throw Error("too many arguments");
            }
            args[argc] = line.substr(n,n2-n);
#ifdef DEBUG
            log->dlog(ch, "arg[%i]: %s", argc, args[argc].c_str());
#endif
            // skip additional ws
            n = line.find_first_not_of(" \t", n2);
            argc++;
        }
 
        // get last argument
        if ((n > 0) && (n < (int) line.length())) {
            if (argc >= (int) (sizeof(args)-1)) {
                throw Error("too many arguments");
            }
            args[argc] = line.substr(n,line.length()-n);
#ifdef DEBUG
            log->dlog(ch, "arg[%i]: %s", argc, args[argc].c_str());
#endif
            argc++;
        }

        if (argc == 0 ) {
            throw Error("parse error");
        }

        // parse the first argument which must be bidname and bidsetname
        n = args[0].find(".");
        if (n > 0) {
            rset = args[0].substr(0, n);
            // use lower case internally
            transform(rset.begin(), rset.end(), rset.begin(), ToLower());
            
            rname = args[0].substr(n+1, tmp.length()-n);
            // use lower case internally
            transform(rname.begin(), rname.end(), rname.begin(), ToLower());
            
        } else {
            rset = "0";
            rname = args[0];
            // use lower case internally
            transform(rname.begin(), rname.end(), rname.begin(), ToLower());
        }

        // parse the rest of the args
        ind = 1;
        while (ind < argc) {
            if ((ind < argc) && (args[ind][0] == '-')) {
                switch (args[ind++][1] ) {
                case 'w':
                    // -wait is not supported
                    break;
                case 'f':
                  {
                      if (ind < argc) {
                          
                          // fields: [<field>=<value> , ...]
                          
                          // bid field of the element
                          while ((ind<argc) && (args[ind][0] != '-')) {
                              field_t f;
                              fieldDefListIter_t iter;
                              
                              // parse param
                              tmp = args[ind];
                              n = tmp.find("=");
                              if ((n > 0) && (n < (int)tmp.length()-1)) {
                                  f.name = tmp.substr(0,n);
                                  // use lower case internally
								  transform(f.name.begin(), f.name.end(), 
												f.name.begin(), ToLower());	
								  
								  // lookup in field definitions list
								  iter = fieldDefs->find(f.name);
								  if (iter != fieldDefs->end()) {
									  // set according to definition
									  f.len = iter->second.len;
									  f.type = iter->second.type;
                                  
									  string fvalue = tmp.substr(n+1, tmp.length()-n);
									  if (fvalue.empty()) {
										  throw Error("Resource Request Parser Error: missing value at line");
									  }
									  // parse and set value
									  try {
										  parseFieldValue(fvalue, &f);
									  } catch(Error &e) {
											throw Error("Resource Request Parser Error: field value parse error at line %s", 
													e.getError().c_str());
									  }
									  
									  fields.push_back(f);
								  } else {
									// else invalid parameter  
									throw Error("field parameter parse error %s", f.name.c_str());
								  }
							  }
                              ind++;
                          }                          
                      }
                  }
                  break;
                case 'i': // Interval
                  {
                      if (ind < argc) 
                      {
                          // only one auction per -a parameter
                          resourceReq_interval_t interval;
                          string name = args[ind++];
                          miscList_t miscList;
                          
                          // bid field of the element
                          while ((ind<argc) && (args[ind][0] != '-')) {
                              configItem_t item;
                              
                              // parse param
							  tmp = args[ind];
							  if (tmp != ",") {
								  n = tmp.find("=");
								  if ((n > 0) && (n < (int)tmp.length()-1)) {
									  item.name = tmp.substr(0,n);
									  item.value = tmp.substr(n+1, tmp.length()-n);
									  item.type = "String";
									  if (item.name == "start") {
										 item.name = "Start";
									  } else if (item.name == "stop") {
										 item.name = "Stop";
									  } else if (item.name == "duration") {
										 item.name = "Duration";
									  } else if (item.name == "interval") {
										 item.name = "Interval";
									  } else {
										 throw Error("unknown option %s", item.name.c_str());
									  }
									  miscList[item.name] = item;

								  } else {
									  // else invalid parameter  
									  throw Error("Resource request parameter parse error");
								  }
							  }  
                              ind++;
                          }        
                          calculateInterval(now, &miscList, &interval);
                          intervals.push_back(interval);
                      }
                      
                  }
                  break;
                default:
                    throw Error(403, "add resource request: unknown option %s", args[ind].c_str() );
                }
            }	
        }
      
        // add resource request
        try {
            ResourceRequest *r = new ResourceRequest(rset, rname, fields, intervals);
            requests->push_back(r);

#ifdef DEBUG
			// debug info
			log->dlog(ch, "Resource request %s.%s - %s", rset.c_str(), rname.c_str(), (r->getInfo()).c_str());
#endif
            
        } catch (Error &e) {
            log->elog(ch, e);
            throw e;
        }
    }
}
