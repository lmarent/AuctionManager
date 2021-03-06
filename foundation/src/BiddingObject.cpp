
/*! \file   BiddingObject.cpp

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
    BiddingObjects in the system - All concrete BiddingObjects inherit from this class.

    $Id: BiddingObject.cpp 748 2015-07-23 15:30:00Z amarentes $
*/


#include <sstream>
#include "ParserFcts.h"
#include "BiddingObject.h"
#include "Error.h"
#include "Timeval.h"

using namespace auction;


/* ------------------------- BiddingObject ------------------------- */

BiddingObject::BiddingObject( string _auctionSet, string _auctionName, string _BiddingObjectSet, string _BiddingObjectName, 
		  ipap_object_type_t _type, elementList_t &elements, optionList_t &options)
  : AuctioningObject("BiddingObject", _BiddingObjectSet, _BiddingObjectName), auctionSet(_auctionSet), auctionName(_auctionName), 
	biddingObjectType(_type), elementList(elements), optionList(options)
{

	if ((_type < IPAP_BID) || (_type > IPAP_ALLOCATION)){
		throw Error("An invalid type was given");
	}

#ifdef DEBUG
    log->dlog(ch, "BiddingObject constructor");
#endif    

}

BiddingObject::BiddingObject( const BiddingObject &rhs )
  : AuctioningObject(rhs)
{

	uid = rhs.uid;
	auctionSet = rhs.auctionSet;
	auctionName = rhs.auctionName;
	biddingObjectType = rhs.biddingObjectType;
	
	// Copy elements part of the BiddingObject.
	elementListConstIter_t iter;
	for (iter = (rhs.elementList).begin(); iter != (rhs.elementList).end(); ++iter ){
		fieldList_t fieldList;
		fieldListconstIter_t fielditer;
		
		for ( fielditer = (iter->second).begin(); 
				fielditer != (iter->second).end(); ++fielditer ){
			field_t field = *fielditer;
			fieldList.push_back(field);			
		}
		elementList[iter->first] = fieldList;
	}
	
	// Copy options part of the BiddingObject.
	optionListConstIter_t iterOpt;
	for (iterOpt = (rhs.optionList).begin(); iterOpt != (rhs.optionList).end(); ++iterOpt ){
		fieldList_t fieldList;
		fieldListconstIter_t fielditer;
		
		for ( fielditer = (iterOpt->second).begin(); 
				fielditer != (iterOpt->second).end(); ++fielditer ){
			field_t field = *fielditer;
			fieldList.push_back(field);
		}
		string optionItem = iterOpt->first;
		optionList.push_back(pair<string, fieldList_t>(optionItem,fieldList));
	}	
}

BiddingObject::~BiddingObject()
{

#ifdef DEBUG
    log->dlog(ch, "BiddingObject destructor %s.%s", getSet().c_str(), getName().c_str());
#endif 
	
	while (!optionList.empty())
	{
		optionList.pop_back();
	}
  
}

string BiddingObject::getInfo()
{
	std::stringstream output;

	output << AuctioningObject::getInfo();

	output << "auctionSet:" << getAuctionSet() 
		   << " auctionName:" << getAuctionName();

	output << "BiddingObjectSet:" << getSet() 
		   << " BiddingObjectName:" << getName();

	output << "type:" << getType();
	
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

string BiddingObject::getIpApId(int domain)
{

	// Set BiddingObject Id.
	string idBiddingObjectS;
	if (getSet().empty()){
		ostringstream ssA;
		ssA << domain;
		idBiddingObjectS =  ssA.str() + "." + getName();
	} else {
		idBiddingObjectS = getSet() + "." + getName();
	}
	
	return idBiddingObjectS;
}

/*! \short   get the Id for the auction that this BiddingObject belongs for using when transfer the BiddingObject in a ipap_message
*/	
string BiddingObject::getAuctionIpAPId()
{
	
	return getAuctionSet() + "." + getAuctionName();	

}

bool BiddingObject::operator==(const BiddingObject &rhs)
{

#ifdef DEBUG
    log->dlog(ch, "Starting operator == ");
#endif  

	if (AuctioningObject::equals(rhs) == false )
		return false;

	if (auctionSet.compare(rhs.auctionSet) != 0 )
		return false;
		
	if (auctionName.compare(rhs.auctionName) != 0 )
		return false;


#ifdef DEBUG
    log->dlog(ch, "operator == equal BiddingObject general info");
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

bool BiddingObject::operator!=(const BiddingObject &param)
{
	return !(operator==(param));
}

field_t 
BiddingObject::getElementVal(string elementName, string name)
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
BiddingObject::getOptionVal(string optionName, string name)
{

#ifdef DEBUG
    log->dlog(ch, "starting getOptionVal OptName:%s fieldname:%s", 
					optionName.c_str(), name.c_str());
#endif
	
	// Convert to lower case for comparison.
	transform(name.begin(), name.end(), name.begin(), ToLower());
	
	field_t field;
	
	optionListIter_t iter;
	for (iter = optionList.begin(); iter != optionList.end(); ++iter ){
		// Convert the record name to lower case
				
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


void BiddingObject::prepare_insert_biddingObjectHdr(pqxx::connection_base &c)
{
	c.prepare("insertBO_HDR", "INSERT INTO biddingObjectHdr( auctionSet, auctionName, BiddingObjectSet, BiddingObjectName, sessionId, biddingObjectType, biddingobjectstatus) VALUES ($1, $2, $3, $4, $5, $6, $7 )");
}

string BiddingObject::execute_insert_biddingObjectHdr( void )
{

	std::string biddingObjectTypeStr = ipap_object_type_names[getType()];
	std::string biddingtypeStateStr = AuctionObjectStateNames[getState()];

	string sentence =  "INSERT INTO biddingObjectHdr( auctionSet, auctionName, BiddingObjectSet,";
	sentence = sentence + " BiddingObjectName, sessionId, biddingObjectType, biddingobjectstatus) VALUES (";
	sentence = sentence + "'" + auctionSet + "',";
	sentence = sentence + "'" + auctionName + "',";
	sentence = sentence + "'" + getSet() + "',";
	sentence = sentence + "'" + getName() + "',";
	sentence = sentence + "'" + sessionId + "',";
	sentence = sentence + "'" + biddingObjectTypeStr + "',";
	sentence = sentence + "'" + biddingtypeStateStr + "'";
	sentence = sentence + ")";

	return sentence;

}

void BiddingObject::prepare_insert_biddingObjectElement(pqxx::connection_base &c)
{
	c.prepare("insertBO_ELEMENT", "INSERT INTO biddingObjectElement( auctionSet, auctionName, BiddingObjectSet, BiddingObjectName, elementName) VALUES ($1, $2, $3, $4, $5 )");
}

string BiddingObject::execute_insert_biddingObjectElement( string elementName )
{
	string sentence = "INSERT INTO biddingObjectElement( auctionSet, auctionName, BiddingObjectSet,";
	sentence = sentence + " BiddingObjectName, elementName) VALUES (";
	sentence = sentence + "'" + auctionSet + "',";
	sentence = sentence + "'" + auctionName + "',";
	sentence = sentence + "'" + getSet() + "',";
	sentence = sentence + "'" + getName() + "',";
	sentence = sentence + "'" + elementName + "'";
	sentence = sentence + ")";
	
	return sentence;
}

void BiddingObject::prepare_insert_biddingObjectElementField(pqxx::connection_base &c)
{
	c.prepare("insertBO_ELEMENTFIELD", "INSERT INTO biddingObjectElementField(auctionSet, auctionName, BiddingObjectSet, BiddingObjectName, elementName, fieldName, fieldType, len, value) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)");
}

string BiddingObject::execute_insert_biddingObjectElementField( string elementName, auction::field_t field )
{
	string sentence = "INSERT INTO biddingObjectElementField(auctionSet, auctionName, ";
	sentence = sentence + " BiddingObjectSet, BiddingObjectName, elementName, ";
	sentence = sentence + " fieldName, fieldType, len, value) VALUES (";
	sentence = sentence + "'" + auctionSet + "',";
	sentence = sentence + "'" + auctionName + "',";
	sentence = sentence + "'" + getSet() + "',";
	sentence = sentence + "'" + getName() + "',";
	sentence = sentence + "'" + elementName + "',";
	sentence = sentence + "'" + field.name + "',";
	sentence = sentence + "'" + field.type + "',";
	ostringstream olen; 
	olen << field.len;
	sentence = sentence + olen.str() + ",";
	sentence = sentence + "'" + (field.value[0]).getValue() + "'";
	sentence = sentence + ")";

	return sentence;
	
}

void BiddingObject::prepare_insert_biddingObjectOption(pqxx::connection_base &c)
{
	c.prepare("insertBO_OPTION", "INSERT INTO biddingObjectOption(auctionSet, auctionName, BiddingObjectSet, BiddingObjectName, optionName) VALUES ($1, $2, $3, $4, $5)");
}

string BiddingObject::execute_insert_biddingObjectOption( string optionName)
{
	string sentence = "INSERT INTO biddingObjectOption(auctionSet, auctionName, BiddingObjectSet,";
	sentence = sentence + " BiddingObjectName, optionName) VALUES (";
	sentence = sentence + "'" + auctionSet + "',";
	sentence = sentence + "'" + auctionName + "',";
	sentence = sentence + "'" + getSet() + "',";
	sentence = sentence + "'" + getName() + "',";
	sentence = sentence + "'" + optionName + "'";
	sentence = sentence + ")";

	return sentence;
	
}

void BiddingObject::prepare_insert_biddingObjectOptionField(pqxx::connection_base &c)
{
	c.prepare("insertBO_OPTIONFIELD", "INSERT INTO biddingObjectOptionField(auctionSet, auctionName, BiddingObjectSet, BiddingObjectName, optionName, fieldName, fieldType, len, value) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)");
}

string BiddingObject::execute_insert_biddingObjectOptionField( string optionName, auction::field_t field)
{
	string sentence = "INSERT INTO biddingObjectOptionField(auctionSet, auctionName,";
	sentence = sentence + " BiddingObjectSet, BiddingObjectName, optionName, fieldName,";
	sentence = sentence + " fieldType, len, value) VALUES (";
	sentence = sentence + "'" + auctionSet + "',";
	sentence = sentence + "'" + auctionName + "',";
	sentence = sentence + "'" + getSet() + "',";
	sentence = sentence + "'" + getName() + "',";
	sentence = sentence + "'" + optionName + "',";
	sentence = sentence + "'" + field.name + "',";
	sentence = sentence + "'" + field.type + "',";
	ostringstream olen; 
	olen << field.len;
	sentence = sentence + olen.str() + ",";
	sentence = sentence + "'" + (field.value[0]).getValue() + "'";
	sentence = sentence + ")";

	return sentence;
}


void 
BiddingObject::calculateIntervals(time_t now, biddingObjectIntervalList_t *list)
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
		
		duration = 0;
		biddingObjectInterval_t biddingObjectInterval;
		biddingObjectInterval.start = 0;
		biddingObjectInterval.stop = 0;
		
		
		field_t fstart = getOptionVal(iter->first, "Start");
		field_t fstop = getOptionVal(iter->first, "Stop");
		field_t fduration = getOptionVal(iter->first, "BiddingDuration");				

#ifdef DEBUG
    log->dlog(ch, "BiddingObject: %s.%s - fstart %s", getSet().c_str(), 
					getName().c_str(), (fstart.getInfo()).c_str());

    log->dlog(ch, "BiddingObject: %s.%s - fstop %s", getSet().c_str(), 
					getName().c_str(), (fstop.getInfo()).c_str());

    log->dlog(ch, "BiddingObject: %s.%s - fduration %s", getSet().c_str(), 
					getName().c_str(), (fduration.getInfo()).c_str());
					
#endif

		if ( (fstart.mtype == FT_WILD)  && 
				(fstop.mtype == FT_WILD) && 
					(fduration.mtype == FT_WILD) ) {
			throw Error(409, "illegal to specify: start+stop+duration time");
		}

		if (fstart.mtype != FT_WILD) {
			string sstart = ((fstart.value)[0]).getString();
			biddingObjectInterval.start = ParserFcts::parseTime(sstart);
			if(biddingObjectInterval.start == 0) {
				throw Error(410, "invalid start time %s", sstart.c_str());
			}
		}

		if (fstop.mtype != FT_WILD) {
			string sstop = ((fstop.value)[0]).getString();
			biddingObjectInterval.stop = ParserFcts::parseTime(sstop);
			if(biddingObjectInterval.stop == 0) {
				throw Error(411, "invalid stop time %s", sstop.c_str());
			}
		}
		
		if (fduration.mtype != FT_WILD) {
			string sduration = ((fduration.value)[0]).getString();
			duration = ParserFcts::parseULong(sduration);
		}

		if ( duration > 0) {
			if (biddingObjectInterval.stop) {
				// stop + duration specified
				biddingObjectInterval.start = biddingObjectInterval.stop - duration;
			} else {
				// stop [+ start] specified
				
				if (biddingObjectInterval.start) {
					biddingObjectInterval.stop = biddingObjectInterval.start + duration;
				} else {
					biddingObjectInterval.start = laststop;
					biddingObjectInterval.stop = biddingObjectInterval.start + duration;
				}
			}
		}

#ifdef DEBUG
    log->dlog(ch, "BiddingObject: %s.%s - now:%s stop %s", getSet().c_str(), 
					getName().c_str(), Timeval::toString(now).c_str(),
					Timeval::toString(biddingObjectInterval.stop).c_str());					
#endif

		// now start has a defined value, while stop may still be zero 
		// indicating an infinite rule
			
		// do we have a stop time defined that is in the past ?
		if ((biddingObjectInterval.stop != 0) && (biddingObjectInterval.stop <= now)) {
			log->dlog(ch, "Bidding object running time is already over");
		}
		
		if (biddingObjectInterval.start < now) {
			// start late tasks immediately
			biddingObjectInterval.start = now;
		}

		laststart = biddingObjectInterval.start;
		laststop = biddingObjectInterval.stop;
		
		list->push_back(pair<time_t,biddingObjectInterval_t>(biddingObjectInterval.start, biddingObjectInterval));
    }    
		
}

void BiddingObject::save_ver4(pqxx::connection_base &c)
{

#ifdef DEBUG
    log->dlog(ch, "Start Save BiddingObject: %s.%s", getSet().c_str(), 
										getName().c_str());
#endif

	
	// Create a transaction object.
	pqxx::work w(c);

	try{
		prepare_insert_biddingObjectHdr(c);
		prepare_insert_biddingObjectElement(c);
		prepare_insert_biddingObjectElementField(c);
		prepare_insert_biddingObjectOption(c);
		prepare_insert_biddingObjectOptionField(c);
		
#ifdef DEBUG
		log->dlog(ch, "after preparing" );
#endif
		
		
		std::string biddingObjectTypeStr = ipap_object_type_names[getType()];
		std::string biddingtypeStateStr = AuctionObjectStateNames[getState()];

		pqxx::result r = w.prepared("insertBO_HDR")(auctionSet)(auctionName)(getSet())(getName())(sessionId)(biddingObjectTypeStr)(biddingtypeStateStr).exec();
		
#ifdef DEBUG
		log->dlog(ch, "after header" );
#endif
		
		elementListIter_t iter;
		for (iter = elementList.begin(); iter != elementList.end(); ++iter ){

			r = w.prepared("insertBO_ELEMENT")(auctionSet)(auctionName)(getSet())(getName())(iter->first).exec();
			fieldListIter_t fieldIter;
			
			for (fieldIter = (iter->second).begin(); fieldIter != iter->second.end(); ++fieldIter)
			{
				r = w.prepared("insertBO_ELEMENTFIELD")(auctionSet)(auctionName)(getSet())(getName())(iter->first)(fieldIter->name)(fieldIter->type)(fieldIter->len)(((fieldIter->value)[0]).getValue()).exec();
			}

		}
		
#ifdef DEBUG
		log->dlog(ch, "after elements" );
#endif
			
		optionListIter_t iterOpt;
		for (iterOpt = optionList.begin(); iterOpt != optionList.end(); ++iterOpt ){

			r = w.prepared("insertBO_OPTION")(auctionSet)(auctionName)(getSet())(getName())(iterOpt->first).exec();

			fieldListIter_t fieldIter;
		
			for (fieldIter = (iterOpt->second).begin(); fieldIter != iterOpt->second.end(); ++fieldIter)
			{
				r = w.prepared("insertBO_OPTIONFIELD")(auctionSet)(auctionName)(getSet())(getName())(iterOpt->first)(fieldIter->name)(fieldIter->type)(fieldIter->len)(((fieldIter->value)[0]).getValue()).exec();
			}

		}
		
		w.commit();
	
	} catch (const pqxx::pqxx_exception & ex) {
		w.abort();
		throw Error(ex.base().what());
	}

}

void BiddingObject::save_ver3(pqxx::connection_base &c)
{

#ifdef DEBUG
    log->dlog(ch, "Start Save BiddingObject in version3: %s.%s - now:%s stop %s", getSet().c_str(), 
										getName().c_str());
#endif

	
	// Create a transaction object.
	pqxx::work w(c);

	try{
		
		string sql;
		
		sql = execute_insert_biddingObjectHdr();
		pqxx::result r = w.exec(sql);
		
				
#ifdef DEBUG
		log->dlog(ch, "after header" );
#endif
		
		elementListIter_t iter;
		for (iter = elementList.begin(); iter != elementList.end(); ++iter ){
				
			sql = execute_insert_biddingObjectElement(iter->first);
			pqxx::result r = w.exec(sql);
	
			fieldListIter_t fieldIter;
			
			for (fieldIter = (iter->second).begin(); fieldIter != iter->second.end(); ++fieldIter)
			{
				
				sql = execute_insert_biddingObjectElementField(iter->first, *fieldIter);
				pqxx::result r = w.exec(sql);
			}

		}
		
#ifdef DEBUG
		log->dlog(ch, "after elements" );
#endif
			
		optionListIter_t iterOpt;
		for (iterOpt = optionList.begin(); iterOpt != optionList.end(); ++iterOpt ){

			sql = execute_insert_biddingObjectOption(iterOpt->first);
			pqxx::result r = w.exec(sql);

			fieldListIter_t fieldIter;
		
			for (fieldIter = (iterOpt->second).begin(); fieldIter != iterOpt->second.end(); ++fieldIter)
			{
				sql = execute_insert_biddingObjectOptionField(iterOpt->first,*fieldIter);
				pqxx::result r = w.exec(sql);
			}

		}
		
		w.commit();
	
	} catch (const pqxx::pqxx_exception & ex) {
		w.abort();
		throw Error(ex.base().what());
	}

}


void 
BiddingObject::setAuctionSet(string _auctionset)
{ 
	auctionSet = _auctionset; 
}	

string 
BiddingObject::getAuctionSet()
{ 
	return auctionSet; 
}

void 
BiddingObject::setAuctionName(string _auctionName) 
{ 
	auctionName = _auctionName; 
}

string 
BiddingObject::getAuctionName()
{ 
	return auctionName; 
}
