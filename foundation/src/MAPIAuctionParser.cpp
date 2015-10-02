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
    
    parser for API message auction syntax
    $Id: MAPIAuctionParser.cpp 2015-07-24 15:14:00 amarentes $
*/

#include "ParserFcts.h"
#include "MAPIAuctionParser.h"

using namespace std;
using namespace auction;

MAPIAuctionParser::MAPIAuctionParser():
	MAPIIpApMessageParser()
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIAuctionParser");
}

void MAPIAuctionParser::getTemplateFields(ipap_template *templ, 
										  fieldDefList_t *fieldDefs,
										  auctionTemplateFieldList_t *templList)
{

	if (templ != NULL){
		int fields = templ->get_numfields();
		for (int i=0; i < fields; ++i){
			ipap_template_field_t tempField = templ->get_field(i);
			fieldDefItem_t fitem = findField(fieldDefs, 
										tempField.elem.get_field_type().eno, 
										tempField.elem.get_field_type().ftype);
			if (fitem.name.empty()){
				ostringstream s;
				s << "Auction Message Parser: Field eno:" << tempField.elem.get_field_type().eno;
				s << "fType:" << tempField.elem.get_field_type().ftype << "is not parametrized";
				throw Error(s.str());
			} else {
				
				// Find the field in the list, if found update its usage
				auctionTemplateFieldListIter_t templIter;
				templIter = templList->find(fitem.name);
				if (templIter == templList->end()){
					// if not found, create and insert it.
					auctionTemplateField_t templField;
					templField.field = fitem;
					templField.length = tempField.elem.get_field_type().length;
					templList->insert ( pair<string,auctionTemplateField_t>(fitem.name,templField) );
				}
				templIter = templList->find(fitem.name);
				if (templ->get_type()== IPAP_SETID_BID_TEMPLATE) (templIter->second).isBidtemplate = true;
				if (templ->get_type()== IPAP_SETID_ALLOCATION_TEMPLATE) (templIter->second).isAllocTemplate = true;
				if (templ->get_type()== IPAP_OPTNS_BID_TEMPLATE) (templIter->second).isOptBidTemplate = true;
			}
		}
	}
}

miscList_t MAPIAuctionParser::readAuctionData( ipap_template *templ, 
											   fieldDefList_t *fieldDefs,
											   ipap_data_record &record,
											   string &auctionName )
{

	miscList_t miscs;
	fieldDataListIter_t fieldIter;
	for (fieldIter=record.begin(); fieldIter!=record.end(); ++fieldIter){
		ipap_field_key kField = fieldIter->first;
		ipap_value_field dFieldValue = fieldIter->second;
		
		fieldDefItem_t fItem = findField(fieldDefs, kField.get_eno() , kField.get_ftype());
		if ((fItem.name).empty()){
			ostringstream s;
			s << "Auction Message Parser: Field eno:" << kField.get_eno();
			s << "fType:" << kField.get_ftype() << "is not parametrized";
			throw Error(s.str()); 
		}
		else{
			ipap_field field = templ->get_field( kField.get_eno(), kField.get_ftype() );

			// Search the action name field.
			if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDAUCTION)){
				auctionName = field.writeValue(dFieldValue);
			}
			else {
				configItem_t item;
				item.name = fItem.name;
				item.type = fItem.type;
				item.value = field.writeValue(dFieldValue);
				miscs[item.name] = item;
			}
		}
	}
	
	return miscs;
}


configItemList_t MAPIAuctionParser::readMiscAuctionData(ipap_template *templ, 
											  fieldDefList_t *fieldDefs,
											  ipap_data_record &record,
											  string &actionName)
{

	configItemList_t miscs;
	fieldDataListIter_t fieldIter;
	for (fieldIter=record.begin(); fieldIter!=record.end(); ++fieldIter){
		ipap_field_key kField = fieldIter->first;
		ipap_value_field dFieldValue = fieldIter->second;
		
		fieldDefItem_t fItem = findField(fieldDefs, kField.get_eno() , kField.get_ftype());
		if ((fItem.name).empty()){
			ostringstream s;
			s << "Auction Message Parser: Field eno:" << kField.get_eno();
			s << "fType:" << kField.get_ftype() << "is not parametrized";
			throw Error(s.str()); 
		}
		else{
			ipap_field field = templ->get_field( kField.get_eno(), kField.get_ftype() );

			// Search the action name field.
			if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_AUCTIONINGALGORITHMNAME)){
				actionName = field.writeValue(dFieldValue);
			}
			else {
				configItem_t item;
				item.name = fItem.name;
				item.type = fItem.type;
				item.value = field.writeValue(dFieldValue);
				miscs.push_back(item);
			}
		}
	}
	
	return miscs;
}

void MAPIAuctionParser::parse( fieldDefList_t *fieldDefs,
							   ipap_message *message,
							   auctionDB_t *auctions,
							   AuctionIdSource *idSource,
							   ipap_message *messageOut )
{

#ifdef DEBUG
    log->dlog(ch, "Starting parse");
#endif
	try
	{
		time_t now = time(NULL);
		string resource;
		string aname;
		string actionName, auctionName;
		action_t action;
		miscList_t miscs;

		// Read the data auction template.
		ipap_template *templAuct = readTemplate(message, IPAP_SETID_AUCTION_TEMPLATE);
			
		// Read the option data auction template.
		ipap_template *templOptAuct = readTemplate(message, IPAP_OPTNS_AUCTION_TEMPLATE);
		
		// Read other templates.
		ipap_template *templBid = readTemplate(message, IPAP_SETID_BID_TEMPLATE);
		ipap_template *templOptBid = readTemplate(message, IPAP_OPTNS_BID_TEMPLATE);
		ipap_template *templAlloc = readTemplate(message, IPAP_SETID_ALLOCATION_TEMPLATE);

		// Read template fields associated.
		auctionTemplateFieldList_t templFields;
		getTemplateFields(templBid, fieldDefs, &templFields);
		getTemplateFields(templOptBid, fieldDefs, &templFields);
		getTemplateFields(templAlloc, fieldDefs, &templFields);

		
		// Read the record data associated with the data auction template.
		if (templAuct != NULL){
			dataRecordList_t dRecordList = readDataRecords(message, templAuct->get_template_id());
			if (dRecordList.size() > 1){
				throw Error("Auction Message Parser: more than a data template"); 
			} else if (dRecordList.size() == 0){
				throw Error("Auction Message Parser: a data template was not given"); 
			} else {
				miscs = readAuctionData( templAuct, fieldDefs, dRecordList[0], auctionName);
				parseName(auctionName, resource, aname);
			}
		} else{
			throw Error("Auction Message Parser: missing data template data"); 
		}
		
		// Read the option data record template associated with the option data auction template.
		if (templOptAuct != NULL){
			dataRecordList_t dOptRecordList = readDataRecords(message, templOptAuct->get_template_id());
			if (dOptRecordList.size() > 1){
				throw Error("Auction Message Parser: more than a option data template"); 
			} else if (dOptRecordList.size() == 0){
				throw Error("Auction Message Parser: an option data template was not given"); 
			} else {
				
				configItemList_t miscsAct = readMiscAuctionData( templOptAuct, 
							fieldDefs, dOptRecordList[0], actionName);
							
				action.name = actionName;
				action.defaultAct = 1;
				action.conf = miscsAct;
			}		
		} else{
			throw Error("Auction Message Parser: missing option data template data"); 
		}

		Auction *a = new Auction(now, resource, aname, action, miscs, templFields, messageOut );
		auctions->push_back(a);

	#ifdef DEBUG
		log->dlog(ch, "Ending parse");
	#endif
	
	} catch (Error &e) {
         log->elog(ch, e);            
         throw e;
    }
}

/* ------------------------- getMessage ------------------------- */
ipap_message * MAPIAuctionParser::get_ipap_message(Auction *auctionPtr, 
											fieldDefList_t *fieldDefs)
{

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message");
#endif

	uint16_t auctionTemplateId, optAuctionTemplateId, templId; 
	int nfields;
	ipap_message *mes = new ipap_message();
	action_t *action = auctionPtr->getAction();
	
	// Add the data auction template
	nfields = 4;
	auctionTemplateId = mes->new_data_template( nfields, IPAP_SETID_AUCTION_TEMPLATE );
	//put the name
	mes->add_field(auctionTemplateId, 0, IPAP_FT_IDAUCTION);
	// put the starttime
	mes->add_field(auctionTemplateId, 0, IPAP_FT_STARTSECONDS);
	// put the endtime
	mes->add_field(auctionTemplateId, 0, IPAP_FT_ENDSECONDS);
	// put the interval.
	mes->add_field(auctionTemplateId, 0, IPAP_FT_INTERVALSECONDS);

#ifdef DEBUG
    log->dlog(ch, "get_ipap_message - after insert data auction template");
#endif
	
	//----------------- Add the option data auction template
	nfields = 2;
	configItemListIter_t actItmiter;
	for (actItmiter= action->conf.begin(); actItmiter!=action->conf.end(); ++actItmiter)
	{
		// Search the field in the list of fields.
		fieldDefItem_t fItem = findField(fieldDefs, actItmiter->name);
		if (fItem.name.empty()){
			ostringstream s;
			s << "Auction err: field definition not found" << actItmiter->name;
			throw Error(s.str());
		} else{
			ipap_field fieldAct = mes->get_field_definition(fItem.eno, fItem.ftype);
			nfields = nfields + 1; 
		}
	}	

	optAuctionTemplateId = mes->new_data_template( nfields, IPAP_OPTNS_AUCTION_TEMPLATE );
	// put the recordId
	mes->add_field(optAuctionTemplateId, 0, IPAP_FT_IDRECORD);
	// put the procname.
	mes->add_field(optAuctionTemplateId, 0, IPAP_FT_AUCTIONINGALGORITHMNAME);

	for (actItmiter= action->conf.begin(); actItmiter!=action->conf.end(); ++actItmiter)
	{
		// Search the field in the list of fields.
		fieldDefItem_t fItem = findField(fieldDefs, actItmiter->name);
		ipap_field fieldAct = mes->get_field_definition(fItem.eno, fItem.ftype);
		mes->add_field(optAuctionTemplateId, fItem.eno, fItem.ftype);
	}
	


	//---------------- Add other templates
	std::list<int> listIds = auctionPtr->getTemplateList()->get_template_list();
	std::list<int>::iterator ids_iterator;
	
	for (ids_iterator = listIds.begin(); ids_iterator != listIds.end(); ++ids_iterator){
		ipap_template *templ = auctionPtr->getTemplateList()->get_template((uint16_t) *ids_iterator);
		int numfields = templ->get_numfields();
		templId = mes->new_data_template( numfields, templ->get_type() );
		for (int i = 0; i < numfields; ++i){
			ipap_template_field_t field = templ->get_field(i);
			mes->add_field(templId, field.elem.get_field_type().eno, 
								field.elem.get_field_type().ftype);
		}
	}

#ifdef DEBUG
    log->dlog(ch, "get_ipap_message - after inserting all other templates");
#endif
	
	string idAuctionS = auctionPtr->getSetName() + auctionPtr->getAuctionName();
	// Add the data record template associated with the data auction template
	ipap_field idAuctionF = mes->get_field_definition( 0, IPAP_FT_IDAUCTION );
	ipap_value_field fvalue0 = idAuctionF.get_ipap_value_field( 
								strdup(idAuctionS.c_str()), idAuctionS.size() );
	
	ipap_data_record data(auctionTemplateId);
	data.insert_field(0, IPAP_FT_IDAUCTION, fvalue0);

	// Add the start time.
	assert (sizeof(uint64_t) >= sizeof(time_t));
	time_t time = auctionPtr->getStart();
	uint64_t timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_field idStartF = mes->get_field_definition( 0, IPAP_FT_STARTSECONDS );
	ipap_value_field fvalue2 = idStartF.get_ipap_value_field( timeUint64 );
	data.insert_field(0, IPAP_FT_STARTSECONDS, fvalue2);

	// Add the end time.
	ipap_field idStopF = mes->get_field_definition( 0, IPAP_FT_ENDSECONDS );
	time = auctionPtr->getStop();
	timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_value_field fvalue3 = idStopF.get_ipap_value_field( timeUint64 );
	data.insert_field(0, IPAP_FT_ENDSECONDS, fvalue3);

	// Add the interval.
	assert (sizeof(uint64_t) >= sizeof(unsigned long));
	uint64_t uinter = static_cast<uint64_t>(auctionPtr->getInterval().interval);
	ipap_field idIntervalF = mes->get_field_definition( 0, IPAP_FT_INTERVALSECONDS );
	ipap_value_field fvalue4 = idIntervalF.get_ipap_value_field( uinter );
	data.insert_field(0, IPAP_FT_INTERVALSECONDS, fvalue4);
	
	// Include data to the message.
	mes->include_data(auctionTemplateId, data);

#ifdef DEBUG
    log->dlog(ch, "get_ipap_message - after inserting data records");
#endif
	
	// Add the option data record template associated with the option data auction template
	int recordIndex = 1;

	ipap_data_record dataOpt(optAuctionTemplateId);

	// Add the Record Id.
	ipap_field idRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	ostringstream ss;
	ss << "Record_" << recordIndex;
	string recordId = ss.str();
	ipap_value_field fvalue1 = idRecordIdF.get_ipap_value_field( 
									strdup(recordId.c_str()), recordId.size() );
	dataOpt.insert_field(0, IPAP_FT_IDRECORD, fvalue1);
			
	
	// Add the action.
	ipap_field idActionF =  mes->get_field_definition(0, IPAP_FT_AUCTIONINGALGORITHMNAME);
	ipap_value_field fvalue5 = idActionF.get_ipap_value_field( strdup(action->name.c_str()), 
										action->name.size() );
	dataOpt.insert_field(0, IPAP_FT_AUCTIONINGALGORITHMNAME, fvalue5);

#ifdef DEBUG
    log->dlog(ch, "get_ipap_message - added the algorithname");
#endif
	
	for (actItmiter= action->conf.begin(); actItmiter!=action->conf.end(); ++actItmiter)
	{
		// Search the field in the list of fields, previously it was verified its existance.
		fieldDefItem_t fItem = findField(fieldDefs, actItmiter->name);

#ifdef DEBUG
		log->dlog(ch, "get_ipap_message - it is going to add %s eno:%d ftype:%d", 
					actItmiter->name.c_str(), fItem.eno, fItem.ftype);
#endif
		ipap_field fieldAct = mes->get_field_definition(fItem.eno, fItem.ftype);
		ipap_value_field actFvalue = fieldAct.parse(actItmiter->value);
		dataOpt.insert_field(fItem.eno, fItem.ftype, actFvalue);
	}	
	mes->include_data(optAuctionTemplateId, dataOpt);
	

#ifdef DEBUG
    log->dlog(ch, "get_ipap_message finished");
#endif	

	return mes;
}

vector<ipap_message *> 
MAPIAuctionParser::get_ipap_messages(fieldDefList_t *fieldDefs, 
									auctionDB_t *auctions)
{

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_messages");
#endif	
	
	vector<ipap_message *> vct_return;
	auctionDBIter_t auctionIter;
	for (auctionIter=auctions->begin(); auctionIter!=auctions->end(); ++auctionIter){
		Auction *a = *auctionIter;
		ipap_message *mes =get_ipap_message(a, fieldDefs);
		vct_return.push_back(mes);
	}

#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_messages");
#endif
	
	return vct_return;
}						
