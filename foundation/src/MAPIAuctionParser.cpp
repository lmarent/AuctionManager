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

MAPIAuctionParser::MAPIAuctionParser(int domain): 
	IpApMessageParser(domain), anslp_ipap_message_splitter()
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIAuctionParser");
}

void 
MAPIAuctionParser::getTemplateFields(ipap_template *templ, 
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
				
				// Find the field in the list, if found insert templates' belongings.
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
				((templIter->second).fieldBelongTo).insert(pair<ipap_object_type_t,ipap_templ_type_t> 
									(ipap_template::getObjectType(templ->get_type()), templ->get_type()  ));
				
			}
		}
	}
}

miscList_t 
MAPIAuctionParser::readAuctionData( ipap_template *templ, 
  								    fieldDefList_t *fieldDefs,
									ipap_data_record &record,
									string &auctionName, 
									string &resourceId,
									string &status,
									string &templateList )
{

#ifdef DEBUG
	log->dlog(ch, "Starting readAuctionData");
#endif


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
				  (kField.get_ftype()== IPAP_FT_IDAUCTION )){
				auctionName = field.writeValue(dFieldValue);
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_AUCTIONINGOBJECTSTATUS )){
				status = field.writeValue(dFieldValue);
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDRESOURCE )){
				resourceId = field.writeValue(dFieldValue);
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype() == IPAP_FT_TEMPLATES )){
				templateList = field.writeValue(dFieldValue);
			} else {
				configItem_t item;
				item.name = fItem.name;
				item.type = fItem.type;
				item.value = field.writeValue(dFieldValue);
				miscs[item.name] = item;
			}
		}
	}

	if (resourceId.empty()){
		throw Error("Resource was not provided"); 
	}
	
	if (status.empty()){
		throw Error("Undetermined auction status"); 
	}

#ifdef DEBUG
	log->dlog(ch, "Ending readAuctionData: Name:%s resourceId:%s Status:%s", auctionName.c_str(),
							resourceId.c_str(), status.c_str());
#endif
	
	return miscs;
}


configItemList_t 
MAPIAuctionParser::readMiscAuctionData(ipap_template *templ, 
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

#ifdef DEBUG
	log->dlog(ch, "Field Read name:%s value:%s", item.name.c_str(), item.value.c_str());
#endif
				
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
		templatesOut->add_template(templData->copy());
		templData2  = templatesOut->get_template(templData->get_template_id());
	}
	
	try {
		templOption2 = templatesOut->get_template(templOption->get_template_id());
	} catch (Error &e){
		templatesOut->add_template(templOption->copy());
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
	string resourceId;
	string sname, aname;
	string actionName, auctionName;
	string status;
	string templateList; 
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
					// This line is used for updating templData and templOption
					ipap_template *templ = findTemplate(templData, templOption, templatesOut, templId );
					
					// Read a data record for a data template 
					if (templData != NULL){
						if (templId == templData->get_template_id()){
							miscs = readAuctionData( templData, fieldDefs, *dataRecordIter, 
													  auctionName, resourceId, status, templateList);
							parseName(auctionName, sname, aname);
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
					throw Error("MAPI Auction Parser: more than a data template "); 
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
				
		a = new Auction(now, sname, aname, resourceId, action, miscs, 
							AS_COPY_TEMPLATE, templFields, templatesOut );

		int istatus = ParserFcts::parseInt(status, 0, 16);
		a->setState((AuctioningObjectState_t) istatus);
		
		// Set templates attached to this auction.
		stringstream ss( templateList.c_str() );
		list<int> templateIds;

		while( ss.good() )
		{
			string substr;
			getline( ss, substr, ',' );
			templateIds.push_back( atoi(substr.c_str()) );
		}
		list<int>::iterator tempIter;
		
		for ( tempIter = templateIds.begin(); tempIter != templateIds.end(); ++tempIter ){
			
			ipap_template *templ = templatesOut->get_template((uint16_t) *tempIter);
			
			a->setBiddingObjectTemplate(ipap_template::getObjectType(templ->get_type()),
										templ->get_type(), templ->get_template_id());
		}
		
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
		
		// Insert the templates associated with any key field auction
		// Those are templates asociated with auctions.
		list<int> templateIds = templates.get_template_list();
		list<int>::iterator templIter; 
		for (templIter = templateIds.begin(); templIter != templateIds.end(); ++templIter ){
			ipap_template *templ = templates.get_template(*templIter);
			// search the template in the output container 
			try{
				
				ipap_template * templ2 = templatesOut->get_template(*templIter);
				assert(*templ == *templ2);
				
			} catch (ipap_bad_argument &e){
				// Insert the template.
				templatesOut->add_template(templ->copy());
			}
		}
	
		anslp::msg::xmlDataRecordIterList_t iter;
		for (iter = objectDataRecords.begin(); iter != objectDataRecords.end();  ++iter)
		{
			if ((iter->first).get_object_type() == IPAP_AUCTION){
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
void MAPIAuctionParser::get_ipap_message(fieldDefList_t *fieldDefs,
										 Auction *auctionPtr, 
										 ipap_template_container *templates,
										 int domainId, bool useIPV6, string sAddressIPV4, 
										 string sAddressIPV6, uint16_t port, ipap_message *mes)
{

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message");
#endif

	char *valueChr=NULL;

	ipap_field_container g_ipap_fields;
	
    g_ipap_fields.initialize_forward();

    g_ipap_fields.initialize_reverse();


	uint16_t auctionTemplateId, optAuctionTemplateId; 
	action_t *action = auctionPtr->getAction();
	
	// Search in the template associated with the auction. If they exist
	// then, we use them.
	
	auctionTemplateId = auctionPtr->getDataAuctionTemplate();
	
	ipap_template *aucTempl = templates->get_template(auctionTemplateId);
	mes->make_template(aucTempl);
		
#ifdef DEBUG
    log->dlog(ch, "get_ipap_message - after insert data auction template numfields:%d", aucTempl->get_numfields());
#endif

	
	//----------------- Add the option data auction template

	optAuctionTemplateId = auctionPtr->getOptionAuctionTemplate();

#ifdef DEBUG
    log->dlog(ch, "template Option Id:%d", optAuctionTemplateId);
#endif

	ipap_template *optTempl = templates->get_template(optAuctionTemplateId)->copy();

	/* Following lines are used for inserting only non mandatory fields */
	set<ipap_field_key> optMandatory = 
		ipap_template::getTemplateTypeMandatoryFields(IPAP_OPTNS_AUCTION_TEMPLATE);
	
	
	set<ipap_field_key> optAuct;
	set<ipap_field_key>::iterator iter;
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
			// Search the field in the set.
			iter = optMandatory.find(ipap_field_key(fItem.eno, fItem.ftype));
			if (iter == optMandatory.end()){

#ifdef DEBUG
			log->dlog(ch, "get_ipap_message - it is going to add %s eno:%d ftype:%d", 
					actItmiter->name.c_str(), fItem.eno, fItem.ftype);
#endif				
				
				// Check that the field is a valid field.
				ipap_field fieldAct = mes->get_field_definition(fItem.eno, fItem.ftype);
				optAuct.insert(ipap_field_key(fItem.eno, fItem.ftype));
			}
		}
	}		

#ifdef DEBUG
	log->dlog(ch, "number of fields in opt template:%d", 
					optTempl->get_numfields() + optAuct.size() );
#endif	
    
    optTempl->set_maxfields(optTempl->get_numfields() + optAuct.size());
    	
    int encodeNetwork = 1;
    set<ipap_field_key>::iterator fieldIter;
    for (fieldIter = optAuct.begin(); fieldIter != optAuct.end(); ++fieldIter)
    {
		ipap_field field = g_ipap_fields.get_field(fieldIter->get_eno(), 
									fieldIter->get_ftype());
									
		uint16_t length = (uint16_t) field.get_field_type().length;
		optTempl->add_field(length,KNOWN,encodeNetwork,field);	
   	}
   	mes->make_template(optTempl);	
	
	// Insert templates associated with the auction.
	for (int i=1; i < IPAP_MAX_OBJECT_TYPE; i++){
		set<ipap_templ_type_t> setObject = ipap_template::getObjectTemplateTypes((ipap_object_type_t) i);
		set<ipap_templ_type_t>::iterator itTemplObject;
		for (itTemplObject = setObject.begin(); itTemplObject != setObject.end(); ++itTemplObject){
			uint16_t templateId = auctionPtr->getBiddingObjectTemplate((ipap_object_type_t) i, *itTemplObject);
			ipap_template *templ = templates->get_template(templateId);
			mes->make_template(templ);
		}
	}
			

#ifdef DEBUG
    log->dlog(ch, "get_ipap_message - after inserting all other templates");
#endif

	// Add the option data record template associated with the option data auction template
	int recordIndex = 1;
	ostringstream ss;
	ss << "Record_" << recordIndex;
	string recordId = ss.str();


	// Add the data record template associated with the data auction template
	ipap_field idAuctionF = mes->get_field_definition( 0, IPAP_FT_IDAUCTION );
	valueChr = strdup( auctionPtr->getIpApId(getDomain()).c_str() );
	ipap_value_field fvalue0 = idAuctionF.get_ipap_value_field( valueChr,  auctionPtr->getIpApId(getDomain()).size() );
	free(valueChr);
	
	ipap_data_record data(auctionTemplateId);
	data.insert_field(0, IPAP_FT_IDAUCTION, fvalue0);

	// Add the Record Id
	ipap_field idRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	valueChr = strdup(recordId.c_str());
	ipap_value_field fvalue1 = idRecordIdF.get_ipap_value_field( valueChr, recordId.size() );
	free(valueChr);
	data.insert_field(0, IPAP_FT_IDRECORD, fvalue1);

	// Add the Status
	ipap_field idStatusF = mes->get_field_definition( 0, IPAP_FT_AUCTIONINGOBJECTSTATUS );
	uint16_t state = (uint16_t)  auctionPtr->getState();
	ipap_value_field fvalStatus = idStatusF.get_ipap_value_field( state );
	data.insert_field(0, IPAP_FT_AUCTIONINGOBJECTSTATUS, fvalStatus);

	// Add the IPversion
	ipap_field ipVersionF = mes->get_field_definition( 0, IPAP_FT_IPVERSION );
	if (useIPV6){
		uint8_t ipversion = 6;
		ipap_value_field fvalueIp = ipVersionF.get_ipap_value_field(ipversion);
		data.insert_field(0, IPAP_FT_IPVERSION, fvalueIp);
	}
	else{
		uint8_t ipversion = 4;
		ipap_value_field fvalueIp = ipVersionF.get_ipap_value_field(ipversion);
		data.insert_field(0, IPAP_FT_IPVERSION, fvalueIp);
	}	
	
	// Add the Ipv6 Address value
	ipap_field ipAddr6F = mes->get_field_definition( 0, IPAP_FT_DESTINATIONIPV6ADDRESS );
	if (useIPV6){	
		ipap_value_field fvalueIpAddr6 = ipAddr6F.parseIP6ADDR(sAddressIPV6);
		data.insert_field(0, IPAP_FT_DESTINATIONIPV6ADDRESS, fvalueIpAddr6);
	}
	else{
		ipap_value_field fvalueIpAddr6 = ipAddr6F.parseIP6ADDR("0:0:0:0:0:0:0:0");
		data.insert_field(0, IPAP_FT_DESTINATIONIPV6ADDRESS, fvalueIpAddr6);	
	}

	// Add the Ipv4 Address value
	ipap_field ipAddr4F = mes->get_field_definition( 0, IPAP_FT_DESTINATIONIPV4ADDRESS );
	if (useIPV6){	
		ipap_value_field fvalueIpAddr4 = ipAddr4F.parseIP4ADDR("0.0.0.0");
		data.insert_field(0, IPAP_FT_DESTINATIONIPV4ADDRESS, fvalueIpAddr4);		
	}
	else{
		ipap_value_field fvalueIpAddr4 = ipAddr4F.parseIP4ADDR(sAddressIPV4);
		data.insert_field(0, IPAP_FT_DESTINATIONIPV4ADDRESS, fvalueIpAddr4);
	}
	
	// Add the destination port
	ipap_field portF = mes->get_field_definition( 0, IPAP_FT_DESTINATIONAUCTIONPORT );
	ipap_value_field fvaluePort = portF.get_ipap_value_field( port);
	data.insert_field(0, IPAP_FT_DESTINATIONAUCTIONPORT, fvaluePort);

	// Add the resource Id. 
	ipap_field resourceIdF = mes->get_field_definition( 0, IPAP_FT_IDRESOURCE );
	ipap_value_field fvalueresourceId = resourceIdF.parseString(  auctionPtr->getAuctionResource() );
	data.insert_field(0, IPAP_FT_IDRESOURCE, fvalueresourceId);
	
	// Add the start time.
	assert (sizeof(uint64_t) >= sizeof(time_t));
	time_t time = auctionPtr->getStart();
	uint64_t timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_field idStartF = mes->get_field_definition( 0, IPAP_FT_STARTSECONDS );
	ipap_value_field fvalueStart = idStartF.get_ipap_value_field( timeUint64 );
	data.insert_field(0, IPAP_FT_STARTSECONDS, fvalueStart);

	// Add the end time.
	ipap_field idStopF = mes->get_field_definition( 0, IPAP_FT_ENDSECONDS );
	time = auctionPtr->getStop();
	timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_value_field fvalueStop = idStopF.get_ipap_value_field( timeUint64 );
	data.insert_field(0, IPAP_FT_ENDSECONDS, fvalueStop);

	// Add the interval.
	assert (sizeof(uint64_t) >= sizeof(unsigned long));
	uint64_t uinter = static_cast<uint64_t>(auctionPtr->getInterval().interval);
	ipap_field idIntervalF = mes->get_field_definition( 0, IPAP_FT_INTERVALSECONDS );
	ipap_value_field fvalueInterval = idIntervalF.get_ipap_value_field( uinter );
	data.insert_field(0, IPAP_FT_INTERVALSECONDS, fvalueInterval);

	// Add the template list.
	string tList = auctionPtr->getTemplateList();
	ipap_field tListF = mes->get_field_definition( 0, IPAP_FT_TEMPLATES );
	ipap_value_field fTList = tListF.parseString( tList );
	data.insert_field(0, IPAP_FT_TEMPLATES, fTList);
	

	// Include data to the message.
	mes->include_data(auctionTemplateId, data);
	
	saveDelete(optTempl);
	
#ifdef DEBUG
    log->dlog(ch, "get_ipap_message - after inserting data records");
#endif

#ifdef DEBUG
    log->dlog(ch, "get_ipap_message - Starting to write down the option data");
#endif

	ipap_data_record dataOpt(optAuctionTemplateId);

	// Add the auction Id
	dataOpt.insert_field(0, IPAP_FT_IDAUCTION, fvalue0);

	// Add the Record Id.
	ipap_field optRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	valueChr = strdup(recordId.c_str());
	ipap_value_field fvalue5 = optRecordIdF.get_ipap_value_field( valueChr , recordId.size() );
	free(valueChr);
	dataOpt.insert_field(0, IPAP_FT_IDRECORD, fvalue5);
						
	// Add the action.
	ipap_field idActionF =  mes->get_field_definition(0, IPAP_FT_AUCTIONINGALGORITHMNAME);
	valueChr = strdup(action->name.c_str());
	ipap_value_field fvalue6 = idActionF.get_ipap_value_field( valueChr, action->name.size() );
	free(valueChr);
	dataOpt.insert_field(0, IPAP_FT_AUCTIONINGALGORITHMNAME, fvalue6);
	
	set<ipap_field_key> optionFields = ipap_template::getTemplateTypeMandatoryFields(IPAP_OPTNS_AUCTION_TEMPLATE);	
	for (actItmiter= action->conf.begin(); actItmiter!=action->conf.end(); ++actItmiter)
	{
		// Search the field in the list of fields, previously it was verified its existance.
		fieldDefItem_t fItem = findField(fieldDefs, actItmiter->name);

		iter = optionFields.find(ipap_field_key(fItem.eno, fItem.ftype));
		if (iter == optionFields.end()){

#ifdef DEBUG
		log->dlog(ch, "get_ipap_message - it is going to add %s eno:%d ftype:%d", 
					actItmiter->name.c_str(), fItem.eno, fItem.ftype);
#endif

			ipap_field fieldAct = mes->get_field_definition(fItem.eno, fItem.ftype);
			ipap_value_field actFvalue = fieldAct.parse(actItmiter->value);
			dataOpt.insert_field(fItem.eno, fItem.ftype, actFvalue);
		}
	}	
	mes->include_data(optAuctionTemplateId, dataOpt);
	

#ifdef DEBUG
    log->dlog(ch, "get_ipap_message finished");
#endif	

}

ipap_message *
MAPIAuctionParser::get_ipap_message(fieldDefList_t *fieldDefs, 
									auctionDB_t *auctions, 
									ipap_template_container *templates,
									bool useIPV6, string sAddressIPV4, 
									string sAddressIPV6, uint16_t port)
{

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message");
#endif	
	
	ipap_message *mes = new ipap_message(getDomain(), IPAP_VERSION, true);
	
	auctionDBIter_t auctionIter;
	for (auctionIter=auctions->begin(); auctionIter!=auctions->end(); ++auctionIter)
	{
		Auction *a = *auctionIter;
		get_ipap_message(fieldDefs, a, templates,  
						getDomain(), useIPV6, sAddressIPV4, sAddressIPV6, port, mes);
	}
	mes->output();
	
	
#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_message");
#endif
	
	return mes;
}						
