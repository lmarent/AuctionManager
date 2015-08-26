
/*!  \file   MAPIAuctionParser.cpp

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
    parser for API text auction syntax

    $Id: MAPIAuctionParser.cpp 2015-07-24 15:14:00 amarentes $
*/

#include "MAPIAuctionParser.h"


MAPIAuctionParser::MAPIAuctionParser(string filename)
    : fileName(filename)
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIAuctionParser" );
}


MAPIAuctionParser::MAPIAuctionParser(char *b, int l)
    : buf(b), len(l)
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIAuctionParser" );
}


// FIXME to be rewritten
void MAPIAuctionParser::parse(auctionDB_t *auctions,
						  AuctionIdSource *idSource )
{

    string sname, rname;
    miscList_t miscs;
    action_t action;
    bool actionDefined = false;
    istringstream in(buf);
    string args[128];
    int argc = 0;
    int ind = 0;
    string tmp;
    int n = 0, n2 = 0;
    string line;
    time_t now = time(NULL);

    // each line contains 1 rule
    while (getline(in, line)) {    
        // tokenize rule string

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

        // parse the first argument which must be auctionname and auctionsetname
        n = args[0].find(".");
        if (n > 0) {
            sname = args[0].substr(0, n);
            // use lower case internally
            transform(sname.begin(), sname.end(), sname.begin(), ToLower());
            
            rname = args[0].substr(n+1, tmp.length()-n);
            
            // use lower case internally
            transform(rname.begin(), rname.end(), rname.begin(), ToLower());
        } else {
            sname = "0";
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
                case 'a':
                  {
                      if (ind < argc ) {
						  if (!actionDefined)
						  {
							  actionDefined = true;
							  // action: <name> [<param>=<value> , ...]
							  action.name = args[ind++];
							  
							  // action parameters
							  while ((ind<argc) && (args[ind][0] != '-')) {
								  configItem_t item;
								  
								  // parse param
								  tmp = args[ind];
								  n = tmp.find("=");
								  if ((n > 0) && (n < (int)tmp.length()-1)) {
									  item.name = tmp.substr(0,n);
									  item.value = tmp.substr(n+1, tmp.length()-n);
									  item.type = "String";
									  // hack: if parameter method = <method> change name to name_<method>
									  // and do not add this parameter
									  if (item.name == "method") {
										  action.name = action.name + "_" + item.value;
									  } else {
										  action.conf.push_back(item);
									  }
								  } else {
									  // else invalid parameter  
									  throw Error("action parameter parse error");
								  }

								  ind++;
							  }
						   }
						   else{
							  // An auction can only have an action.
							  throw Error("action already defined");
						   }
                      }
                  }
                  break;
                case 'm':
                    while ((ind<argc) && (args[ind][0] != '-')) {
                        configItem_t item;

                        tmp = args[ind];
                        // skip the separating commas
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

                                miscs[item.name] = item;
                            } else {
                                throw Error("misc parse error");
                            }
                        }                       

                        ind++;
                    }
                    break;
                default:
                    throw Error(403, "add_auction: unknown option %s", args[ind].c_str() );
                }
            }	
        }
  
#ifdef DEBUG
        // debug info
        log->dlog(ch, "auction %s.%s", sname.c_str(), rname.c_str());
        log->dlog(ch, " A %s", action.name.c_str());
        for (configItemListIter_t j = action.conf.begin(); j != action.conf.end(); j++) {
            log->dlog(ch, "  C %s = %s", j->name.c_str(), j->value.c_str());
        }
        for (miscListIter_t i = miscs.begin(); i != miscs.end(); i++) {
            log->dlog(ch, " C %s = %s", i->second.name.c_str(), i->second.value.c_str());
        }
#endif
    
        // add rule
        try {
            Auction *a = new Auction(now, sname, rname, action, miscs);
            auctions->push_back(a);
        } catch (Error &e) {
            log->elog(ch, e);
            throw e;
        }
    }
}
