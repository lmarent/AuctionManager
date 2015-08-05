
/*!\file   Auction.cpp

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    his file is part of Network Auction Manager System (NETAUM).

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
    container class for auction

    $Id: Auction.cpp 748 2015-08-04 10:13:00Z amarentes $

*/

#include "Auction.h"
#include "Error.h"
#include "ParserFcts.h"
#include "Constants.h"

/* ------------------------- Auction ------------------------- */

Auction::Auction(time_t now, string sname, string rname, action_t &a,  miscList_t &m)
  : uid(-1), state(RS_NEW), ruleName(rname), setName(sname), 
		 action(a), miscList(m)
{
    unsigned long duration;

    log = Logger::getInstance();
    ch = log->createChannel("Auction");

#ifdef DEBUG
    log->dlog(ch, "Auction constructor");
#endif    

    try {
	
        parseAuctionName(rname);
	
        if (rname.empty()) {
            // we tolerate an empty sname but not an empty rname
            throw Error("missing auction identifier value in auction description");
        }
        if (sname.empty()) {
            sname = DEFAULT_SETNAME;
        }

        /* time stuff */
        start = now;
        // stop = 0 indicates infinite running time
        stop = 0;
        // duration = 0 indicates no duration set
        duration = 0;
	    
        // get the configured values
        string sstart = getMiscVal("Start");
        string sstop = getMiscVal("Stop");
        string sduration = getMiscVal("Duration");
        string sinterval = getMiscVal("Interval");
        string salign = getMiscVal("Align");
	    
        if (!sstart.empty() && !sstop.empty() && !sduration.empty()) {
            throw Error(409, "illegal to specify: start+stop+duration time");
        }
	
        if (!sstart.empty()) {
            start = parseTime(sstart);
            if(start == 0) {
                throw Error(410, "invalid start time %s", sstart.c_str());
            }
        }

        if (!sstop.empty()) {
            stop = parseTime(sstop);
            if(stop == 0) {
                throw Error(411, "invalid stop time %s", sstop.c_str());
            }
        }
	
        if (!sduration.empty()) {
            duration = ParserFcts::parseULong(sduration);
        }

        if (duration) {
            if (stop) {
                // stop + duration specified
                start = stop - duration;
            } else {
                // stop [+ start] specified
                stop = start + duration;
            }
        }
	
        // now start has a defined value, while stop may still be zero 
        // indicating an infinite rule
	    
        // do we have a stop time defined that is in the past ?
        if ((stop != 0) && (stop <= now)) {
            throw Error(300, "auction running time is already over");
        }
	
        if (start < now) {
            // start late auctions immediately
            start = now;
        }

        int interval = 0;
        if (!sinterval.empty()) {
            interval = ParserFcts::parseInt(sinterval);
        }
        
        int align = (!salign.empty()) ? 1 : 0;

		if (interval > 0) {
			// add to intervals list
			interval_t ientry;

			ientry.interval = interval;
			ientry.align = align;

			intervals.push_back(ientry);
        }

    } catch (Error &e) {    
        state = RS_ERROR;
        throw Error("Auction %s.%s: %s", sname.c_str(), rname.c_str(), e.getError().c_str());
    }
}


/* ------------------------- ~Auction ------------------------- */

Auction::~Auction()
{
#ifdef DEBUG
    log->dlog(ch, "Auction destructor");
#endif    

}

/* functions for accessing the templates */

string Auction::getMiscVal(string name)
{
    miscListIter_t iter;

    iter = miscList.find(name);
    if (iter != miscList.end()) {
        return iter->second.value;
    } else {
        return "";
    }
}


void Auction::parseRuleName(string rname)
{
    int n;

    if (rname.empty()) {
        throw Error("malformed rule identifier %s, "
                    "use <identifier> or <source>.<identifier> ",
                    rname.c_str());
    }

    if ((n = rname.find(".")) > 0) {
        resource = rname.substr(0,n);
        id = rname.substr(n+1, rname.length()-n);
    } else {
        // no dot so everything is recognized as id
        id = rname;
    }

}


/* ------------------------- parseTime ------------------------- */

time_t Auction::parseTime(string timestr)
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


/* ------------------------- getAction ------------------------- */

action_t *Auction::getAction()
{
    return &action;
}


/* ------------------------- getMisc ------------------------- */

miscList_t *Auction::getMisc()
{
    return &miscList;
}


/* ------------------------- dump ------------------------- */

void Auction::dump( ostream &os )
{
    os << "Auction dump :" << endl;
    os << getInfo() << endl;
  
}


/* ------------------------- getInfo ------------------------- */

string Auction::getInfo(void)
{
    ostringstream s;

    s << getSetName() << "." << getRuleName() << " ";

    switch (getState()) {
    case RS_NEW:
        s << "new";
        break;
    case RS_VALID:
        s << "validated";
        break;
    case RS_SCHEDULED:
        s << "scheduled";
        break;
    case RS_ACTIVE:
        s << "active";
        break;
    case RS_DONE:
        s << "done";
        break;
    case RS_ERROR:
        s << "error";
        break;
    default:
        s << "unknown";
    }

    s << ": ";


    s << " | ";
	
	s << ai->name;
    s << ", ";

    s << " | ";

    miscListIter_t mi = miscList.begin();
    while (mi != miscList.end()) {
        s << mi->second.name << " = " << mi->second.value;

        mi++;

        if (mi != miscList.end()) {
            s << ", ";
        }
    }

    s << endl;

    return s.str();
}

/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, Auction &ri )
{
    ri.dump(os);
    return os;
}

