
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
#include "Timeval.h"
#include "IpAp_def.h"
#include "IpAp_message.h"
#include "TemplateIdSource.h"
#include "IpApMessageParser.h"

using namespace auction;

/* ------------------------- Auction ------------------------- */

Auction::Auction(time_t now, string sname, string aname, string resource, action_t &a, 
				 miscList_t &m, AuctionTemplateMode_t mode, 
				 auctionTemplateFieldList_t &templFields,
				 ipap_template_container *templates )
   : AuctioningObject("AUCTION"), auctionName(aname), resource(resource), setName(sname), 
   	 action(a), miscList(m)
{

    unsigned long duration;

    
#ifdef DEBUG
    log->dlog(ch, "Auction constructor");
#endif    

    try {
		
        if (aname.empty()) {
            // we tolerate an empty set name but not an empty auction name
            throw Error("missing auction identifier value in rule description");
        }
		
        /* time stuff */
        start = now;
        // stop = 0 indicates infinite running time
        stop = 0;
        // duration = 0 indicates no duration set
        duration = 0;
	    
        // get the configured values
        string sstart = IpApMessageParser::getMiscVal(getMisc(), "start");
        string sstop = IpApMessageParser::getMiscVal(getMisc(), "stop");
        string sduration = IpApMessageParser::getMiscVal(getMisc(), "duration");
        string sinterval = IpApMessageParser::getMiscVal(getMisc(), "interval");
        string salign = IpApMessageParser::getMiscVal(getMisc(), "align");

#ifdef DEBUG
		log->dlog(ch, "SStart:%s SStop:%s SDuration:%s SInterval:%s, SAlign:%s", 
					sstart.c_str(), sstop.c_str(), sduration.c_str(),
					sinterval.c_str(), salign.c_str());
#endif

	    
        if (!sstart.empty() && !sstop.empty() && !sduration.empty()) {
            throw Error(409, "illegal to specify: start+stop+duration time");
        }
	
        if (!sstart.empty()) {
            start = ParserFcts::parseTime(sstart);
            if(start == 0) {
                throw Error(410, "invalid start time %s", sstart.c_str());
            }
        }

        if (!sstop.empty()) {
            stop = ParserFcts::parseTime(sstop);
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
        if ((stop != 0) && (stop <= now)) 
        {
#ifdef DEBUG
			log->dlog(ch, "auction running time is already over");
#endif 			
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
    log->dlog(ch, "Start:%s End:%s Interval: %d - Align:%d", 
					(Timeval::toString(start)).c_str(), 
						(Timeval::toString(stop)).c_str(),
							interval, align);
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
        state = AO_ERROR;
        throw Error("Auction %s.%s: %s", sname.c_str(), aname.c_str(), e.getError().c_str());
    }

}

Auction::Auction(const Auction &rhs): 
	AuctioningObject(rhs), auctionName(rhs.auctionName), setName(rhs.setName), 
	 resource(rhs.resource)
{
	
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

	// Copy references to sessions 
	sessionListIter_t session_it;
	for (session_it = rhs.sessions.begin(); session_it != rhs.sessions.end(); ++session_it)
	{
		sessions.insert(*session_it);
	}
	
}

string 
Auction::getIpApId(int domain)
{
	string idAuctionS;
	if ((getSetName()).empty()){
		ostringstream ssA;
		ssA << domain;
		idAuctionS =  ssA.str() + "." + getAuctionName();
	} else {
		idAuctionS = getSetName() + "." + getAuctionName();
	}
	
	return idAuctionS;
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
Auction::calculateTemplateFields( ipap_object_type_t objectType,
								  ipap_templ_type_t templType, 
								  auctionTemplateFieldList_t &templFields)
{
    
    // 1. insert the template mandatory fields.
    set<ipap_field_key> tFields = ipap_template::getTemplateTypeMandatoryFields(templType);
        
    // 2. insert other configurated fields.
    auctionTemplateFieldListIter_t fieldIter;
    for (fieldIter = templFields.begin(); fieldIter != templFields.end();  ++fieldIter)
    {
		bool include = false;
		
		auctionTemplateField_t field = fieldIter->second;
		set< pair<ipap_object_type_t,ipap_templ_type_t> >::iterator iter;		
		for (iter = (field.fieldBelongTo).begin(); iter != (field.fieldBelongTo).end(); ++iter) {
			if ( (iter->first == objectType) && (iter->second == templType) ) {
				include = true;
				break;
			}
		}
		
		if (include) {
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
Auction::createAuctionTemplate(ipap_field_container g_ipap_fields,
							   ipap_templ_type_t templType )
{

	TemplateIdSource *tId = TemplateIdSource::getInstance();

    set<ipap_field_key> tFields = ipap_template::getTemplateTypeMandatoryFields(templType);
    		
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

ipap_template * 
Auction::createBiddingObjectTemplate(auctionTemplateFieldList_t &templFields,
									 ipap_field_container g_ipap_fields,
									 ipap_object_type_t objectType,
									 ipap_templ_type_t templType )
{

	TemplateIdSource *tId = TemplateIdSource::getInstance();

    set<ipap_field_key> tFields = calculateTemplateFields(objectType, templType, templFields);
    		
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
	ipap_template *auctTemplate = createAuctionTemplate(g_ipap_fields, IPAP_SETID_AUCTION_TEMPLATE);
								
	setDataAuctionTemplate(auctTemplate->get_template_id());
	templateContainer->add_template(auctTemplate);
	
	// Creates the option auction template
	ipap_template *optAuctTemplate = createAuctionTemplate(g_ipap_fields, IPAP_OPTNS_AUCTION_TEMPLATE);
								
	setOptionAuctionTemplate(optAuctTemplate->get_template_id()); 
	templateContainer->add_template(optAuctTemplate);
	
	// Insert other templates related to bidding objects.
	int i = 0;
	for ( i = 1; i < IPAP_MAX_OBJECT_TYPE; i++ ){
		
		
		set<ipap_templ_type_t> templSet = ipap_template::getObjectTemplateTypes((ipap_object_type_t) i);
		set<ipap_templ_type_t>::iterator iter;
		for (iter = templSet.begin(); iter != templSet.end(); ++iter ){
		
			ipap_template *templ = 
				createBiddingObjectTemplate(templFields, g_ipap_fields,
											 (ipap_object_type_t) i, *iter );
											 
			// Add a local reference to the template.
			setBiddingObjectTemplate((ipap_object_type_t) i, *iter, templ->get_template_id());
	
			// Insert the template in the general container.
			templateContainer->add_template(templ);
		}
	}
	
#ifdef DEBUG
	log->dlog(ch, "Nbr templates :%d", ((i-1)*2));
#endif  


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
	
/* ------------------- getDataBiddingObjectTemplate ---------------------- */
uint16_t Auction::getBiddingObjectTemplate(ipap_object_type_t type, ipap_templ_type_t templType)
{
	auctionTemplateListIter_t ret = biddingObjectTemplates.find(type);
	
	if (ret != biddingObjectTemplates.end()) {
		map<ipap_templ_type_t, uint16_t>::iterator iter;
		iter = (ret->second).find(templType);
		if ( iter != (ret->second).end() ) {
			return iter->second;
		} 
		else {
			return 0;
		}
	}
	return 0;
	
}
	
/* ------------------- setDataBidTemplate ---------------------- */
void Auction::setBiddingObjectTemplate(ipap_object_type_t type, 
						ipap_templ_type_t templType, uint16_t templId)
{
		
	biddingObjectTemplates[type][templType] = templId;
	
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

	s << AuctioningObject::getInfo();

    s << getSetName() << "." << getAuctionName() << " ";

	s << getStart() << " & " << getStop() << " ";


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

string 
Auction::getTemplateList(void)
{
	ostringstream s;
	bool firstTime = true;
	
	for (int i=1; i < IPAP_MAX_OBJECT_TYPE; i++){
		set<ipap_templ_type_t> setObject = ipap_template::getObjectTemplateTypes((ipap_object_type_t) i);
		set<ipap_templ_type_t>::iterator itTemplObject;
		for (itTemplObject = setObject.begin(); itTemplObject != setObject.end(); ++itTemplObject){
			uint16_t templateId = getBiddingObjectTemplate((ipap_object_type_t) i, *itTemplObject);
			if (firstTime == true){
				s << templateId;
				firstTime = false;
			} else {
				s << "," << templateId;
			}	
		}
	}
	
	return s.str();
}

void Auction::incrementSessionReferences(string sessionId)
{
	sessions.insert(sessionId);
}
	
	//! decrement the number of session references to this auction
void Auction::decrementSessionReferences(string sessionId)
{
	sessionListIter_t it = sessions.find(sessionId);
	sessions.erase(it);
}
