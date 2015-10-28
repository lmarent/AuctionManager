
/*! \file   Bid.cpp

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
    Bids in the system - All concrete bids inherit from this class.

    $Id: Bid.cpp 748 2015-07-23 15:30:00Z amarentes $
*/


#include <sstream>
#include "ParserFcts.h"
#include "Bid.h"
#include "Error.h"
#include "Timeval.h"

using namespace auction;


/* ------------------------- Bid ------------------------- */

Bid::Bid( string auctionSet, string auctionName, string bidSet, string bidName, 
		  elementList_t &elements, optionList_t &options)
  : uid(0), state(BS_NEW), auctionSet(auctionSet), auctionName(auctionName), 
	bidSet(bidSet), bidName(bidName), elementList(elements), optionList(options)
{

    log = Logger::getInstance();
    ch = log->createChannel("Bid");

#ifdef DEBUG
    log->dlog(ch, "Bid constructor");
#endif    

}

Bid::Bid( const Bid &rhs )
  : uid(0), state(BS_NEW)
{

    log = Logger::getInstance();
    ch = log->createChannel("Bid");

	uid = rhs.uid;
	auctionSet = rhs.auctionSet;
	auctionName = rhs.auctionName;
	bidSet = rhs.bidSet;
	bidName = rhs.bidName;
	
	// Copy elements part of the bid.
	elementListConstIter_t iter;
	for (iter = (rhs.elementList).begin(); iter != (rhs.elementList).end(); ++iter ){
		fieldList_t fieldList;
		fieldListconstIter_t fielditer;
		
		for ( fielditer = (iter->second).begin(); 
				fielditer != (iter->second).end(); ++fielditer ){
			field_t field;
			field.name = (*fielditer).name;
			field.type = (*fielditer).type;
			field.mtype = (*fielditer).mtype;
			field.len = (*fielditer).len;
			field.cnt = (*fielditer).cnt;
			
			// Initialize the values for the field.
			for (int i=0 ; i < MAX_FIELD_SET_SIZE; i++)
			{
				FieldValue fielvalue;
				field.value.push_back(fielvalue);
			}

			// Assign the values from the bid.
			for (int i=0 ; i< MAX_FIELD_SET_SIZE; ++i) {
				field.value[i] = FieldValue((*fielditer).value[i]);
			}
			fieldList.push_back(field);
		}
		elementList[iter->first] = fieldList;
	}
	
	// Copy options part of the bid.
	optionListConstIter_t iterOpt;
	for (iterOpt = (rhs.optionList).begin(); iterOpt != (rhs.optionList).end(); ++iterOpt ){
		fieldList_t fieldList;
		fieldListconstIter_t fielditer;
		
		for ( fielditer = (iterOpt->second).begin(); 
				fielditer != (iterOpt->second).end(); ++fielditer ){
			field_t field;
			field.name = (*fielditer).name;
			field.type = (*fielditer).type;
			field.mtype = (*fielditer).mtype;
			field.len = (*fielditer).len;
			field.cnt = (*fielditer).cnt;
			
			// Initialize the values for the field.
			for (int i=0 ; i < MAX_FIELD_SET_SIZE; i++)
			{
				FieldValue fielvalue;
				field.value.push_back(fielvalue);
			}

			// Assign the values from the bid.
			for (int i=0 ; i< MAX_FIELD_SET_SIZE; ++i) {
				field.value[i] = FieldValue((*fielditer).value[i]);
			}
			fieldList.push_back(field);
		}
		optionList.push_back(pair<string, fieldList_t>(iterOpt->first,fieldList));
	}	
}

Bid::~Bid()
{
#ifdef DEBUG
    log->dlog(ch, "Bid destructor");
#endif    

}

string Bid::getInfo()
{
	std::stringstream output;

	output << "auctionSet:" << getAuctionSet() 
		   << " auctionName:" << getAuctionName();

	output << "bidSet:" << getBidSet() 
		   << " bidName:" << getBidName();

	output 	<< " NbrElementLists:" << elementList.size()
			<< std::endl;
	
	elementListIter_t iter;
	for (iter = elementList.begin(); iter != elementList.end(); ++iter ){

		output << "Element Name:" << iter->first << std::endl;

		fieldListIter_t fieldIter;
		
		output << " Field number:" << iter->second.size() << std::endl;
		for (fieldIter = (iter->second).begin(); fieldIter != iter->second.end(); ++fieldIter)
		{
			output << "field name:" << fieldIter->name 
				   << " type:" << fieldIter->type
				   << " len:" << fieldIter->len
				   << " Value:" << ((fieldIter->value)[0]).getValue()
				   << std::endl;
		}

	}

	output 	<< " NbrOptionLists:" << optionList.size()
			<< std::endl;
			
	optionListIter_t iterOpt;
	for (iterOpt = optionList.begin(); iterOpt != optionList.end(); ++iterOpt ){

		output << "Option Name:" << iterOpt->first << std::endl;

		fieldListIter_t fieldIter;
		
		output << " Field number:" << iterOpt->second.size() << std::endl;
		for (fieldIter = (iterOpt->second).begin(); fieldIter != iterOpt->second.end(); ++fieldIter)
		{
			output << "field name:" << fieldIter->name 
				   << " type:" << fieldIter->type
				   << " len:" << fieldIter->len
				   << " Value:" << ((fieldIter->value)[0]).getValue()
				   << std::endl;
		}

	}
	
	return output.str();
}

bool Bid::operator==(const Bid &rhs)
{

#ifdef DEBUG
    log->dlog(ch, "Starting operator == ");
#endif  

	if (auctionSet.compare(rhs.auctionSet) != 0 )
		return false;
		
	if (auctionName.compare(rhs.auctionName) != 0 )
		return false;

	if (bidSet.compare(rhs.bidSet) != 0 )
		return false;

	if (bidName.compare(rhs.bidName) != 0 )
		return false;


#ifdef DEBUG
    log->dlog(ch, "operator == equal bid general info");
#endif  

	if (elementList.size() != rhs.elementList.size())
		return false;
		
	elementListIter_t iter;
	for (iter = elementList.begin(); iter != elementList.end(); ++iter ){
		// Look for the same element in the rhs object
		elementListConstIter_t elementConstIter = rhs.elementList.find(iter->first);
		if (elementConstIter == rhs.elementList.end()){			
			return false;
		} else {
			
			try{
				for (size_t i = 0; i < (iter->second).size(); ++i ){
					// Look for the field in the rhs with the name.
					
					field_t ls = (iter->second)[i];
					field_t rs;
					for (size_t j = 0; j < (elementConstIter->second).size(); ++j){
						if (ls.name == (elementConstIter->second)[j].name){
							rs = (elementConstIter->second)[j];
							break;
						}
					}
					
					if ( ls != rs){
						return false;
					}
				}		
			} catch (out_of_range &e){
				return false;
			}
		}
	}
	
#ifdef DEBUG
    log->dlog(ch, "operator == equal elements info");
#endif


	if (optionList.size() != rhs.optionList.size())
		return false;
		
	optionListIter_t iterOpt;
	for (iterOpt = optionList.begin(); iterOpt != optionList.end(); ++iterOpt ){
		
		// Look for the same option in the rhs object
		optionListConstIter_t optionConstIter;
		for (optionConstIter = rhs.optionList.begin(); optionConstIter != rhs.optionList.end(); ++optionConstIter){
			if (optionConstIter->first == iterOpt->first){
				break;
			}
		}
		
		if (optionConstIter == rhs.optionList.end()){			
			return false;
		} else {
			
			try{
				for (size_t i = 0; i < (iterOpt->second).size(); ++i ){
					// Look for the field in the rhs with the name.
					
					field_t ls = (iterOpt->second)[i];
					field_t rs;
					for (size_t j = 0; j < (optionConstIter->second).size(); ++j){
						if (ls.name == (optionConstIter->second)[j].name){
							rs = (optionConstIter->second)[j];
							break;
						}
					}
					
					if ( ls != rs){
						return false;
					}
				}		
			} catch (out_of_range &e){
				return false;
			}
		}
	}
	
#ifdef DEBUG
    log->dlog(ch, "operator == equal auction info");
#endif
	
	return true;
}

bool Bid::operator!=(const Bid &param)
{
	return !(operator==(param));
}

field_t 
Bid::getElementVal(string elementName, string name)
{
	field_t field;
	elementListIter_t iter = elementList.find(elementName);
	
	if (iter != elementList.end())
	{
		fieldListIter_t fieldIter;
		for (fieldIter = (iter->second).begin(); fieldIter != (iter->second).end(); ++fieldIter){
			if (fieldIter->name == name) {
				return *fieldIter;
			} 
		}
	}
	
	return field;
}

/* functions for accessing the templates */
field_t
Bid::getOptionVal(string optionName, string name)
{

#ifdef DEBUG
    log->dlog(ch, "starting getOptionVal OptName:%s fieldname:%s", 
					optionName.c_str(), name.c_str());
#endif
	
	// Convert to lower case for comparison.
	transform(optionName.begin(), optionName.end(), optionName.begin(), ToLower());
	transform(name.begin(), name.end(), name.begin(), ToLower());
	
	field_t field;
	
	optionListIter_t iter;
	for (iter = optionList.begin(); iter != optionList.end(); ++iter ){
		if (iter->first == optionName){ 
			fieldListIter_t fieldIter;

			for (fieldIter = (iter->second).begin(); fieldIter != (iter->second).end(); ++fieldIter){
				if (fieldIter->name == name) 
					return *fieldIter;
			}
		}
	}
	
	return field;
}


void Bid::calculateIntervals(time_t now, bidIntervalList_t *list)
{

#ifdef DEBUG
    log->dlog(ch, "Start calculate intervals");
#endif
    
    time_t laststart = now;
    time_t laststop = now;  
    unsigned long duration;

	optionListIter_t iter;
	for (iter = optionList.begin(); iter != optionList.end(); ++iter)
	{
		
		bidInterval_t bidInterval;
		bidInterval.start = 0;
		bidInterval.stop = 0;
		
		
		field_t fstart = getOptionVal(iter->first, "Start");
		field_t fstop = getOptionVal(iter->first, "Stop");
		field_t fduration = getOptionVal(iter->first, "BidDuration");				

#ifdef DEBUG
    log->dlog(ch, "bid: %s.%s - fstart %s", getBidSet().c_str(), 
					getBidName().c_str(), (fstart.getInfo()).c_str());

    log->dlog(ch, "bid: %s.%s - fstop %s", getBidSet().c_str(), 
					getBidName().c_str(), (fstop.getInfo()).c_str());

    log->dlog(ch, "bid: %s.%s - fduration %s", getBidSet().c_str(), 
					getBidName().c_str(), (fduration.getInfo()).c_str());
					
#endif

		if ( (fstart.mtype == FT_WILD)  && 
				(fstop.mtype == FT_WILD) && 
					(fduration.mtype == FT_WILD) ) {
			throw Error(409, "illegal to specify: start+stop+duration time");
		}

		if (fstart.mtype != FT_WILD) {
			string sstart = ((fstart.value)[0]).getString();
			bidInterval.start = ParserFcts::parseTime(sstart);
			if(bidInterval.start == 0) {
				throw Error(410, "invalid start time %s", sstart.c_str());
			}
		}

		if (fstop.mtype != FT_WILD) {
			string sstop = ((fstop.value)[0]).getString();
			bidInterval.stop = ParserFcts::parseTime(sstop);
			if(bidInterval.stop == 0) {
				throw Error(411, "invalid stop time %s", sstop.c_str());
			}
		}
		
		if (fduration.mtype != FT_WILD) {
			string sduration = ((fduration.value)[0]).getString();
			duration = ParserFcts::parseULong(sduration);
		}

		if ( duration > 0) {
			if (bidInterval.stop) {
				// stop + duration specified
				bidInterval.start = bidInterval.stop - duration;
			} else {
				// stop [+ start] specified
				
				if (bidInterval.start) {
					bidInterval.stop = bidInterval.start + duration;
				} else {
					bidInterval.start = laststop;
					bidInterval.stop = bidInterval.start + duration;
				}
			}
		}

#ifdef DEBUG
    log->dlog(ch, "bid: %s.%s - now:%s stop %s", getBidSet().c_str(), 
					getBidName().c_str(), Timeval::toString(now).c_str(),
					Timeval::toString(bidInterval.stop).c_str());					
#endif

		// now start has a defined value, while stop may still be zero 
		// indicating an infinite rule
			
		// do we have a stop time defined that is in the past ?
		if ((bidInterval.stop != 0) && (bidInterval.stop <= now)) {
			throw Error(300, "Bid running time is already over");
		}
		
		if (bidInterval.start < now) {
			// start late tasks immediately
			bidInterval.start = now;
		}

		laststart = bidInterval.start;
		laststop = bidInterval.stop;
		
		list->push_back(pair<time_t,bidInterval_t>(bidInterval.start, bidInterval));
    }    
		
}
