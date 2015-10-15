
/*! \file   Auction.cpp

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
    Auctions in the system.

    $Id: Auction.cpp 748 2015-07-23 9:58:00Z amarentes $
*/


#include <sstream>
#include <assert.h>

#include "ParserFcts.h"
#include "Auction.h"
#include "Error.h"
#include "IpAp_def.h"
#include "IpAp_message.h"
#include "TemplateIdSource.h"

using namespace auction;

/* ------------------------- Auction ------------------------- */

Auction::Auction(time_t now, string sname, string aname, action_t &a, 
				 miscList_t &m, AuctionTemplateMode_t mode, 
				 auctionTemplateFieldList_t &templFields,
				 ipap_template_container *templates )
   : state(AS_NEW), auctionName(aname), resource(), setName(sname), 
   	 dataAuctionTemplate(0), optionAuctionTemplate(0), 	dataBidTemplate(0),
	 optionBidTemplate(0), dataAllocationTemplate(0), optionAllocationTemplate(0), 
     action(a), miscList(m)
{

    unsigned long duration;

    log = Logger::getInstance();
    ch = log->createChannel("Auction");
    
#ifdef DEBUG
    log->dlog(ch, "Auction constructor");
#endif    

    try {
	
        parseAuctionName(aname);
	
        if (aname.empty()) {
            // we tolerate an empty sname but not an empty rname
            throw Error("missing auction identifier value in rule description");
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
        string sstart = getMiscVal("start");
        string sstop = getMiscVal("stop");
        string sduration = getMiscVal("duration");
        string sinterval = getMiscVal("interval");
        string salign = getMiscVal("align");
	    
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

#ifdef DEBUG
    log->dlog(ch, "Duration %d", duration);
#endif 

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
            // start late tasks immediately
            start = now;
        }
		
        // get export module params
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
			
#ifdef DEBUG
    log->dlog(ch, "Interval: %d - Align:%d", interval, align);
#endif    
			
           	mainInterval = ientry;
	    }
	    
	    if (mode == AS_BUILD_TEMPLATE){
			buildTemplates(templFields, templates);
		}
		
#ifdef DEBUG
		log->dlog(ch, "Ending Constructor Auction");
#endif 

    } catch(ipap_bad_argument &e){
		throw Error("Auction %s.%s: %s", sname.c_str(), aname.c_str(), e.what());	
    } catch (Error &e) {    
        state = AS_ERROR;
        throw Error("Auction %s.%s: %s", sname.c_str(), aname.c_str(), e.getError().c_str());
    }

}

Auction::Auction(const Auction &rhs): 
	state(rhs.state), auctionName(rhs.auctionName), setName(rhs.setName), 
	 resource(rhs.resource)/*,action(),intervals(), miscList()*/
{

    log = Logger::getInstance();
    ch = log->createChannel("Auction");
	
	start = rhs.start;
	stop = rhs.stop;

	// Copy action
	action.name = rhs.action.name;
	action.defaultAct = rhs.action.defaultAct;
	configItemListConstIter_t iter;
	for (iter = (rhs.action.conf).begin(); iter != (rhs.action.conf).end(); ++iter)
	{
		configItem_t item;
		item.group = iter->group;
		item.module = iter->module;
		item.name = iter->name;
		item.value = iter->value;
		item.type = iter->type;
		action.conf.push_back(item);
	} 
	
	
	// Copy the main interval
	mainInterval = rhs.mainInterval;
		
	// Copy miscList
	miscListConstIter_t misc_it;
	miscList.clear();
	for (misc_it = (rhs.miscList).begin(); misc_it != (rhs.miscList).end(); ++misc_it)
	{
		configItem_t item;
		item.group = (misc_it->second).group;
		item.module = (misc_it->second).module;
		item.name = (misc_it->second).name;
		item.value = (misc_it->second).value;
		item.type = (misc_it->second).type;
		miscList.insert( std::pair<string,configItem_t>(item.name,item) );
	}

	// Copy references to templates 
	// It just maintains references, the template container is the one controlling memory
	
	
}


/* ------------------------- ~Auction ------------------------- */

Auction::~Auction()
{
#ifdef DEBUG
    log->dlog(ch, "Auction destructor Id: %d", uid);
#endif

}

void Auction::addTemplateField(ipap_template *templ, 
							   ipap_field_container g_ipap_fields, 
							   int eno, int type)
{
	// By default network encoding
	int encodeNetwork = 1;
	
	ipap_field field = g_ipap_fields.get_field(eno, type);
	uint16_t length = (uint16_t) field.get_field_type().length;
   	templ->add_field(length,KNOWN,encodeNetwork,field);

}

void Auction::addTemplateMandatoryFields(ipap_template *templ, 
										ipap_field_container g_ipap_fields)
{
	ipap_templ_type_t templType = templ->get_type();
	set<ipap_field_key> keys = ipap_template::getTemplateTypeMandatoryFields(templType);
	
	set<ipap_field_key>::iterator iter;
	for (iter = keys.begin(); iter != keys.end(); ++iter){
		addTemplateField(templ, g_ipap_fields, iter->get_eno(), iter->get_ftype());
	}
	
}

set<ipap_field_key> 
Auction::calculateTemplateFields(ipap_templ_type_t templType, 
									   auctionTemplateFieldList_t &templFields)
{
    
    
    set<ipap_field_key> tFields = ipap_template::getTemplateTypeMandatoryFields(templType);
    
    auctionTemplateFieldListIter_t fieldIter;
    for (fieldIter = templFields.begin(); fieldIter != templFields.end();  ++fieldIter)
    {
		bool include = false;
		
		if ( (fieldIter->second).isBidtemplate 
		     &&  ( templType == IPAP_SETID_BID_TEMPLATE) ){
			include = true;
		}

		if ( (fieldIter->second).isOptBidTemplate 
		     &&  ( templType == IPAP_OPTNS_BID_TEMPLATE) ){
			include = true;
		}

		if ( (fieldIter->second).isAllocTemplate 
		     &&  ( templType == IPAP_SETID_ALLOCATION_TEMPLATE) ){
			include = true;
		}

		if (include){
			ipap_field_key key((fieldIter->second).field.eno, 
							   (fieldIter->second).field.ftype);
			// Exclude keys from the number of fields.
			if (tFields.find(key) == tFields.end()){
				tFields.insert(key);
			}
		}

	} 

	return tFields;
}

ipap_template * 
Auction::createTemplate(auctionTemplateFieldList_t &templFields,
		  			    ipap_field_container g_ipap_fields,
						ipap_templ_type_t templType)
{

	TemplateIdSource *tId = TemplateIdSource::getInstance();

    set<ipap_field_key> tFields = calculateTemplateFields(templType, templFields);
    		
    // Create the bid template associated with the auction
    ipap_template *templ = new ipap_template();
	templ->set_id( tId->newId() );
	templ->set_maxfields( tFields.size() );
	templ->set_type(templType);
    	
    set<ipap_field_key>::iterator fieldIter;
    for (fieldIter = tFields.begin(); fieldIter != tFields.end(); ++fieldIter)
    {
		addTemplateField(templ, g_ipap_fields, fieldIter->get_eno(), fieldIter->get_ftype());
	} 

	return templ;
}

void Auction::buildTemplates(auctionTemplateFieldList_t &templFields, 
							 ipap_template_container *templateContainer)
{

	
	
	ipap_field_container g_ipap_fields;
	
    g_ipap_fields.initialize_forward();

    g_ipap_fields.initialize_reverse();
	
	// Creates the auction data template
	ipap_template *auctTemplate = createTemplate(templFields, 
								g_ipap_fields, IPAP_SETID_AUCTION_TEMPLATE);
	setDataAuctionTemplate(auctTemplate->get_template_id());
	templateContainer->add_template(auctTemplate);
	
	// Creates the option auction template
	ipap_template *optAuctTemplate = createTemplate(templFields, 
								g_ipap_fields, IPAP_OPTNS_AUCTION_TEMPLATE);
	setOptionAuctionTemplate(optAuctTemplate->get_template_id()); 
	templateContainer->add_template(optAuctTemplate);
	
	
	ipap_template *bidTemplate = createTemplate(templFields, 
								g_ipap_fields, IPAP_SETID_BID_TEMPLATE);

	// Add a local reference to the template.
	setDataBidTemplate(bidTemplate->get_template_id());
	
	// Insert the template in the general container.
	templateContainer->add_template(bidTemplate);

	ipap_template *OptBidTemplate = createTemplate(templFields, 
								g_ipap_fields, IPAP_OPTNS_BID_TEMPLATE);

	// Add a local reference to the template.
	setOptionBidTemplate(OptBidTemplate->get_template_id());
	
	// Insert the template in the general container.
	templateContainer->add_template(OptBidTemplate);


	ipap_template *allocTemplate = createTemplate(templFields, 
								g_ipap_fields, IPAP_SETID_ALLOCATION_TEMPLATE);

	// Add a local reference to the template.
	setDataAllocationTemplate(allocTemplate->get_template_id());
	
	// Insert the template in the general container.
	templateContainer->add_template(allocTemplate);
	
#ifdef DEBUG
	log->dlog(ch, "templates: bidId:%d, optBidId:%d, allocId:%d", 
					bidTemplate->get_template_id(),
					OptBidTemplate->get_template_id(),
					allocTemplate->get_template_id());
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


void Auction::parseAuctionName(string rname)
{
    int n;

    if (rname.empty()) {
        throw Error("malformed auction identifier %s, "
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
#ifdef DEBUG
    log->dlog(ch, "Starting parseTime - value given: %s", timestr.c_str());
#endif 	
	
    
  
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
        
        struct tm  t;
        
        if (timestr.empty()){
           return 0;
        } else if (timestr.find_first_not_of("0123456789") == string::npos){
			return (time_t) ParserFcts::parseULLong(timestr);
		} else if (strptime(timestr.c_str(), TIME_FORMAT.c_str(), &t) != NULL){
			return mktime(&t);
        } else{
			return 0;
		}
    }
    
}

/* ------------------------- getActions ------------------------- */


action_t *Auction::getAction()
{
    return &action;
}


/* ------------------------- getMisc ------------------------- */


miscList_t *Auction::getMisc()
{
    return &miscList;
}

/* ------------------- getDataAuctionTemplate ---------------------- */

uint16_t Auction::getDataAuctionTemplate(void)
{
	return dataAuctionTemplate;
}
    
/* ------------------- setDataAuctionTemplate ---------------------- */
void Auction::setDataAuctionTemplate(uint16_t templId)
{
	dataAuctionTemplate = templId;
}
    
/* ------------------- getOptionAuctionTemplate ---------------------- */
uint16_t Auction::getOptionAuctionTemplate(void)
{
	return optionAuctionTemplate;
}
	
/* ------------------- setOptionAuctionTemplate ---------------------- */
void Auction::setOptionAuctionTemplate(uint16_t templId)
{
	optionAuctionTemplate = templId;
}
	
/* ------------------- getDataBidTemplate ---------------------- */
uint16_t Auction::getDataBidTemplate(void)
{
	return dataBidTemplate;
}
	
/* ------------------- setDataBidTemplate ---------------------- */
void Auction::setDataBidTemplate(uint16_t templId)
{
	dataBidTemplate = templId;
}
	
/* ------------------- getOptionBidTemplate ---------------------- */
uint16_t Auction::getOptionBidTemplate(void)
{
	return optionBidTemplate;
}
	
/* ------------------- setOptionBidTemplate ---------------------- */
void Auction::setOptionBidTemplate(uint16_t templId)
{
	optionBidTemplate = templId;
}
	
/* ------------------- getDataAllocationTemplate ------------------ */
uint16_t Auction::getDataAllocationTemplate(void)
{
	return dataAllocationTemplate;
}
	
/* ------------------- setDataAllocationTemplate ------------------ */
void Auction::setDataAllocationTemplate(uint16_t templId)
{
	dataAllocationTemplate = templId;
}
	
/* ------------------- getOptionAllocationTemplate ------------------ */
uint16_t Auction::getOptionAllocationTemplate(void)
{
	return optionAllocationTemplate;
}
 
/* ------------------- setOptionAllocationTemplate ------------------ */
void Auction::setOptionAllocationTemplate(uint16_t templId)
{
	optionAllocationTemplate = templId;
}

/* ------------------------- dump ------------------------- */

void Auction::dump( ostream &os )
{
    os << "Rule dump :" << endl;
    os << getInfo() << endl;
  
}

/* ------------------------- getInfo ------------------------- */

string Auction::getInfo(void)
{
    ostringstream s;

    s << getSetName() << "." << getAuctionName() << " ";

	s << getStart() << " & " << getStop() << " ";

    switch (getState()) {
    case AS_NEW:
        s << "new";
        break;
    case AS_VALID:
        s << "validated";
        break;
    case AS_SCHEDULED:
        s << "scheduled";
        break;
    case AS_ACTIVE:
        s << "active";
        break;
    case AS_DONE:
        s << "done";
        break;
    case AS_ERROR:
        s << "error";
        break;
    default:
        s << "unknown";
    }

    s << ": ";
	
    s << action.name << " default:";
    s << action.defaultAct;

	configItemListIter_t iter;
	for (iter = action.conf.begin(); iter != action.conf.end(); ++iter)
	{
		s << "Group:" << iter->group 
		  << " module:" << iter->module
		  << " name:" << iter->name
		  << " value:" << iter->value
		  << " type:" << iter->type;
	}
	
	
    s << " | ";
	
    miscListIter_t mi = miscList.begin();
    while (mi != miscList.end()) {
        s << mi->second.name << " = " << mi->second.value;

        mi++;

        if (mi != miscList.end()) {
            s << ", ";
        }
    }
	
    s << " | ";


	s << "Interval:" << (mainInterval).interval << "align:" << (mainInterval).align;
			
    s << endl;

    return s.str();
}

