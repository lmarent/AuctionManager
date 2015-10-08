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
#include "xml_object_key.h"
#include "anslp_ipap_xml_message.h"

using namespace std;
using namespace auction;

MAPIAuctionParser::MAPIAuctionParser():
	MAPIIpApMessageParser(), anslp_ipap_message_splitter()
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
				s << "MAPI Auction Parser: Field eno:" << tempField.elem.get_field_type().eno;
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
			s << "MAPI Auction Parser: Field eno:" << kField.get_eno();
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
			s << "MAPI Auction Parser: Field eno:" << kField.get_eno();
			s << "fType:" << kField.get_ftype() << " is not parametrized";
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

void 
MAPIAuctionParser::verifyInsertTemplates(ipap_template *templData, ipap_template *templOption, 
						   ipap_template_container *templatesOut)
{
	// Verifies that both templates have been included
	if (templData == NULL){ 
		return;
	}
		
	if (templOption == NULL){
		return;
	}
	
	ipap_template *templData2 = NULL;	
	ipap_template *templOption2 = NULL;
	
	// Insert templates in case of not created in the template container.
	try {
		templData2  = templatesOut->get_template(templData->get_template_id());
	} catch (Error &e){
		templatesOut->add_template(templData);
		templData2  = templatesOut->get_template(templData->get_template_id());
	}
	
	try {
		templOption2 = templatesOut->get_template(templOption->get_template_id());
	} catch (Error &e){
		templatesOut->add_template(templOption);
		templOption2 = templatesOut->get_template(templOption->get_template_id());
	}
	
	// Verifies that both templates are equal to those 
	// defined in the template container or they do not exist. 
    if ((*templData != *templData2)){
		throw Error("MAPI Auction Parser: Data Template %d given is different from the template already stored", templData->get_template_id()); 
	}

    if ((*templOption != *templOption2)){
		throw Error("MAPI Auction Parser: Option Template %d given is different from the template already stored", templOption->get_template_id()); 
	}

}

ipap_template * 
MAPIAuctionParser::findTemplate(ipap_template *templData, ipap_template *templOption,
				  ipap_template_container *templatesOut, uint16_t templId)
{
	if (templData != NULL){
		if (templData->get_template_id() == templId){
			return templData;
		}
	}

	if (templOption != NULL){
		if (templOption->get_template_id() == templId){
			return templOption;
		}
	}

	try {
		ipap_template *templ = templatesOut->get_template(templId);
		if (templ->get_type() == IPAP_SETID_AUCTION_TEMPLATE){
			templData = templ;
		}
						
		if  (templ->get_type() == IPAP_OPTNS_AUCTION_TEMPLATE){
			templOption = templ;
		}
		
		return templ;
		
	} catch (Error &e) {
		log->elog(ch, e.getError().c_str());
		throw Error("MAPI Auction Parser: missing template  %d", templId); 
	}

}


void MAPIAuctionParser::parseAuctionKey( fieldDefList_t *fieldDefs, 
										 const anslp::msg::xml_object_key &key, 
									     auctionDB_t *auctions,
									     ipap_template_container *templatesOut )
{

	time_t now = time(NULL);
	string resource;
	string aname;
	string actionName, auctionName;
	action_t action;
	miscList_t miscs;

	auctionTemplateFieldList_t templFields;
	ipap_template *templData = NULL;
	ipap_template *templOption = NULL;
	Auction *a = NULL;

#ifdef DEBUG
	log->dlog(ch, "Starting parseAuctionKey %s", key.to_string().c_str());
#endif


	try{
		// Read templates.
		anslp::msg::xmlTemplateIterList_t tmplIter;
		for (tmplIter = objectTemplates.begin(); tmplIter != objectTemplates.end(); ++tmplIter)
		{
			// Correspond to the same key.
			if (tmplIter->first == key){ 
				std::list<int>::iterator tmpIntIterator;
				std::list<int> tempList = (tmplIter->second).get_template_list();
				 
				for (tmpIntIterator = tempList.begin(); tmpIntIterator != tempList.end(); ++tmpIntIterator){ 
					ipap_template *templ = (tmplIter->second).get_template(*tmpIntIterator);
					getTemplateFields(templ, fieldDefs, &templFields);
					
					if (templ->get_type() == IPAP_SETID_AUCTION_TEMPLATE){
						templData = templ;
					}
						
					if  (templ->get_type() == IPAP_OPTNS_AUCTION_TEMPLATE){
						templOption = templ;
					}
				}
			}
		}
		
		// Verifies templates
		verifyInsertTemplates(templData, templOption, templatesOut);
		
		// Read data records.
		anslp::msg::xmlDataRecordIterList_t datIter;
		int NbrDataRead = 0;
		int NbrOptionRead = 0;
		
		for (datIter = objectDataRecords.begin(); datIter != objectDataRecords.end(); ++datIter)
		{
			if (datIter->first == key){ 
				
				dateRecordListIter_t dataRecordIter;
				for (dataRecordIter = (datIter->second).begin(); 
						dataRecordIter != (datIter->second).end(); ++dataRecordIter){
					
					uint16_t templId = dataRecordIter->get_template_id();
					ipap_template *templ = findTemplate(templData, templOption, templatesOut, templId );
					
					// Read a data record for a data template 
					if (templData != NULL){
						if (templId == templData->get_template_id()){
							miscs = readAuctionData( templData, fieldDefs, *dataRecordIter, auctionName);
							parseName(auctionName, resource, aname);
							++NbrDataRead;
						}
					}
					
					// Read a data record for an option template 
					if (templOption != NULL){
						if (templId == templOption->get_template_id()){
							configItemList_t miscsAct = readMiscAuctionData( templOption, 
										fieldDefs, *dataRecordIter, actionName);
										
							action.name = actionName;
							action.defaultAct = 1;
							action.conf = miscsAct;
							NbrOptionRead++;
						}
					}
				}
				
				if (NbrDataRead > 1){
					throw Error("MAPI Auction Parser: more than a data template"); 
				}

				if (NbrDataRead == 0){
					throw Error("MAPI Auction Parser: a data template was not given"); 
				}
				
				if (NbrOptionRead > 1){
					throw Error("MAPI Auction Parser: more than a option data template"); 
				}	

				if (NbrOptionRead == 0){
					throw Error("MAPI Auction Parser: an option template was not given"); 
				}
				// data has been read, so we can finish.
				break;
			}
		}

		a = new Auction(now, resource, aname, action, miscs, 
							AS_COPY_TEMPLATE, templFields, templatesOut );
		auctions->push_back(a);
	
	} catch (Error &e){
		if (a != NULL)
		{
			saveDelete(a);
		}
		throw e;
	}
	
}

void MAPIAuctionParser::parse( fieldDefList_t *fieldDefs,
							   ipap_message *messsage,
							   auctionDB_t *auctions,
							   ipap_template_container *templatesOut )
{

	try
	{

#ifdef DEBUG
		log->dlog(ch, "Starting parse");
#endif

		anslp::msg::anslp_ipap_message mes(*messsage);
				
		split(const_cast< anslp::msg::anslp_ipap_message &>(mes));
		
		anslp::msg::xmlDataFieldKeyIterList_t iter;
		for (iter = objectDataRecordKeys.begin(); iter != objectDataRecordKeys.end();  ++iter)
		{
			if ((iter->first).get_object_type() == anslp::msg::IPAP_AUCTION){
				parseAuctionKey(fieldDefs, iter->first, auctions, templatesOut); 
			}
		}
		

	#ifdef DEBUG
		log->dlog(ch, "Ending parse");
	#endif
	
	
	} catch (Error &e) {
         log->elog(ch, e);            
         throw e;
    }
}

/* ------------------------- getMessage ------------------------- */
void MAPIAuctionParser::get_ipap_message(Auction *auctionPtr, 
											fieldDefList_t *fieldDefs,
											ipap_message *mes)
{

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message");
#endif

	uint16_t auctionTemplateId, optAuctionTemplateId, templId; 
	int nfields;
	action_t *action = auctionPtr->getAction();
	
	// Add the data auction template
	nfields = 5;
	auctionTemplateId = mes->new_data_template( nfields, IPAP_SETID_AUCTION_TEMPLATE );
	//put the name
	mes->add_field(auctionTemplateId, 0, IPAP_FT_IDAUCTION);
	// put the recordId
	mes->add_field(auctionTemplateId, 0, IPAP_FT_IDRECORD);
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
	nfields = 3;
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

	// put the AuctionId
	mes->add_field(optAuctionTemplateId, 0, IPAP_FT_IDAUCTION);
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

	// Add the Record Id
	string dataRecordId = "temporal";
	ipap_field idRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	ipap_value_field fvalue1 = idRecordIdF.get_ipap_value_field( 
									strdup(dataRecordId.c_str()), dataRecordId.size() );
	data.insert_field(0, IPAP_FT_IDRECORD, fvalue1);

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

	// Add the auction Id
	dataOpt.insert_field(0, IPAP_FT_IDAUCTION, fvalue0);

	// Add the Record Id.
	ipap_field optRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	ostringstream ss;
	ss << "Record_" << recordIndex;
	string recordId = ss.str();
	ipap_value_field fvalue5 = optRecordIdF.get_ipap_value_field( 
									strdup(recordId.c_str()), recordId.size() );
	dataOpt.insert_field(0, IPAP_FT_IDRECORD, fvalue5);
			
	
	// Add the action.
	ipap_field idActionF =  mes->get_field_definition(0, IPAP_FT_AUCTIONINGALGORITHMNAME);
	ipap_value_field fvalue6 = idActionF.get_ipap_value_field( strdup(action->name.c_str()), 
										action->name.size() );
	dataOpt.insert_field(0, IPAP_FT_AUCTIONINGALGORITHMNAME, fvalue6);

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

}

ipap_message *
MAPIAuctionParser::get_ipap_message(fieldDefList_t *fieldDefs, 
									auctionDB_t *auctions)
{

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message");
#endif	
	
	ipap_message *mes = new ipap_message();
	
	auctionDBIter_t auctionIter;
	for (auctionIter=auctions->begin(); auctionIter!=auctions->end(); ++auctionIter)
	{
		Auction *a = *auctionIter;
		get_ipap_message(a, fieldDefs, mes);
	}

#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_message");
#endif
	
	return mes;
}						
