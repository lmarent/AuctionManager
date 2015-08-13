
/*!  \file   MAPIBidParser.cpp

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
    parser for API text bid syntax

    $Id: MAPIBidParser.cpp 2015-07-24 15:14:00 amarentes $
*/

#include "MAPIBidParser.h"


MAPIBidParser::MAPIBidParser(string filename)
    : fileName(filename)
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIBidParser" );
}


MAPIBidParser::MAPIBidParser(char *b, int l)
    : buf(b), len(l)
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIBidParser" );
}


string MAPIBidParser::lookup(fieldValList_t *fieldVals, string fvalue, field_t *f)
{
    fieldValListIter_t iter2 = fieldVals->find(fvalue);
    if ((iter2 != fieldVals->end()) && (iter2->second.type == f->type)) {
        // substitute field value
        fvalue = iter2->second.svalue;
    }

    return fvalue;
}


void MAPIBidParser::parseFieldValue(fieldValList_t *fieldVals, string value, field_t *f)
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


void MAPIBidParser::parse(fieldDefList_t *fieldDefs, 
						  fieldValList_t *fieldVals, 
						  bidDB_t *bids,
						  BidIdSource *idSource )
{
    string sname, rname;
    elementList_t elements;
    bidAuctionList_t auctions;
    istringstream in(buf);
    string args[128];
    int argc = 0;
    int ind = 0;
    string tmp;
    int n = 0, n2 = 0;
    string line;

    // each line contains 1 bid
    while (getline(in, line)) {    
        // tokenize rule string

#ifdef DEBUG
            log->dlog(ch, "Line given: %s", line.c_str());
#endif
		
		
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
            sname = args[0].substr(0, n);
            rname = args[0].substr(n+1, tmp.length()-n);
        } else {
            sname = "0";
            rname = args[0];
        }

        // parse the rest of the args
        ind = 1;
        while (ind < argc) {
            if ((ind < argc) && (args[ind][0] == '-')) {
                switch (args[ind++][1] ) {
                case 'w':
                    // -wait is not supported
                    break;
                case 'e':
                  {
                      if (ind < argc) {
                          // only one action per -a parameter
                          element_t elem;
                          
                          // element: <name> [<field>=<value> , ...]
                          elem.name = args[ind++];
                          
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
										  throw Error("Bid Parser Error: missing value at line");
									  }
									  // parse and set value
									  try {
										  parseFieldValue(fieldVals, fvalue, &f);
									  } catch(Error &e) {
											throw Error("Bid Parser Error: field value parse error at line %s", 
													e.getError().c_str());
									  }
									  
									  elem.fields.push_back(f);
								  } else {
									// else invalid parameter  
									throw Error("field parameter parse error %s", f.name.c_str());
								  }
							  }
                              ind++;
                          }
                          
                          elements.push_back(elem);
                      }
                      
                  }
                  break;
                case 'a':
                  {
                      if (ind < argc) 
                      {
                          // only one auction per -a parameter
                          bid_auction_t auction;
                          
                          // auction: <set.name> [<param>=<value> , ...]
                          auction.name = args[ind++];
                          
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
									  auction.miscList[item.name] = item;

								  } else {
									  // else invalid parameter  
									  throw Error("bid-auction parameter parse error");
								  }
							  }  
                              ind++;
                          }        
                          auctions.push_back(auction);
                      }
                      
                  }
                  break;
                default:
                    throw Error(403, "add_bid: unknown option %s", args[ind].c_str() );
                }
            }	
        }
      
        // add bid
        try {
            unsigned short uid = idSource->newId();
            Bid *b = new Bid((int) uid, sname, rname, elements, auctions);
            bids->push_back(b);

#ifdef DEBUG
			// debug info
			log->dlog(ch, "bid %s.%s - %s", sname.c_str(), rname.c_str(), (b->getInfo()).c_str());
#endif
            
        } catch (Error &e) {
            log->elog(ch, e);
            throw e;
        }
    }
}
