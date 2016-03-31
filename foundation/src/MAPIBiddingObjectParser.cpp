
/*!  \file   MAPIBiddingObjectParser.cpp

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
    parser for API message bidding object syntax

    $Id: MAPIBiddingObjectParser.cpp 2015-07-24 15:14:00 amarentes $
*/

#include "ParserFcts.h"
#include "Constants.h"
#include "MAPIBiddingObjectParser.h"
#include "Timeval.h"

using namespace auction;

MAPIBiddingObjectParser::MAPIBiddingObjectParser(int domain)
    : IpApMessageParser(domain)
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIBiddingObjectParser" );
    
#ifdef DEBUG
    log->dlog(ch, "Constructor MAPIBiddingObjectParser");
#endif    
}

												

void MAPIBiddingObjectParser::addDataRecord(fieldDefList_t *fieldDefs, 
											BiddingObject * biddingObjectPtr, string recordId, 
											fieldList_t &fieldList, uint16_t templateId, 
											ipap_message *mes )
{

#ifdef DEBUG
    log->dlog(ch, "Starting addDataRecord");
#endif
	
	char *valueChr=NULL;
	
	ipap_data_record data(templateId);

	// Add the auction Id
	ipap_field idAuctionF = mes->get_field_definition( 0, IPAP_FT_IDAUCTION );
	valueChr = strdup((biddingObjectPtr->getAuctionIpAPId()).c_str());
	ipap_value_field fvalue3 = idAuctionF.get_ipap_value_field( valueChr, 
										( biddingObjectPtr->getAuctionIpAPId()).size() );
	free(valueChr);
										
	data.insert_field(0, IPAP_FT_IDAUCTION, fvalue3);
	
	// Add the BiddingObject Id
	ipap_field idBidF = mes->get_field_definition( 0, IPAP_FT_IDBIDDINGOBJECT );
	valueChr = strdup((biddingObjectPtr->getIpApId(getDomain())).c_str());
	ipap_value_field fvalue1 = idBidF.get_ipap_value_field( valueChr,  
										(biddingObjectPtr->getIpApId(getDomain())).size() );
	free(valueChr);
	data.insert_field(0, IPAP_FT_IDBIDDINGOBJECT, fvalue1);

	// Add the Record Id
	ipap_field idRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	valueChr = strdup(recordId.c_str());
	ipap_value_field fvalue2 = idRecordIdF.get_ipap_value_field( valueChr, recordId.size() );
	free(valueChr);
	data.insert_field(0, IPAP_FT_IDRECORD, fvalue2);	

	// Add the Status
	ipap_field idStatusF = mes->get_field_definition( 0, IPAP_FT_AUCTIONINGOBJECTSTATUS );
	uint16_t state = (uint16_t)  biddingObjectPtr->getState();
	ipap_value_field fvalStatus = idStatusF.get_ipap_value_field( state );
	data.insert_field(0, IPAP_FT_AUCTIONINGOBJECTSTATUS, fvalStatus);	

	// Add the object type.
	ipap_field typeF = mes->get_field_definition( 0, IPAP_FT_BIDDINGOBJECTTYPE );
	uint8_t type = (uint8_t)  biddingObjectPtr->getType();
	ipap_value_field fvalType = typeF.get_ipap_value_field( type );
	data.insert_field(0, IPAP_FT_BIDDINGOBJECTTYPE, fvalType );	

	// Find the option template for this object type
	ipap_templ_type_t tempType = ipap_template::getTemplateType(biddingObjectPtr->getType(), IPAP_RECORD);

	// Only add other fields not include as mandatory fields.
	set<ipap_field_key> recordFields = ipap_template::getTemplateTypeMandatoryFields(tempType);
	set<ipap_field_key>::iterator iter;

	// Add the rest of the field within the record.
	fieldListIter_t fieldListIter;
	for (fieldListIter = fieldList.begin(); fieldListIter != fieldList.end(); ++fieldListIter)
	{
		// Search the field in the list of fields, previously it was verified its existance.
		fieldDefItem_t fItem = findField(fieldDefs, fieldListIter->name);

		iter = recordFields.find(ipap_field_key(fItem.eno, fItem.ftype));
		if (iter == recordFields.end()){

#ifdef DEBUG
			log->dlog(ch, "get_ipap_message - it is going to add %s eno:%d ftype:%d", 
					fieldListIter->name.c_str(), fItem.eno, fItem.ftype);
#endif
			ipap_field fieldAct = mes->get_field_definition(fItem.eno, fItem.ftype);
		
			//TODO AM: we have to fix to manage more than one value.
			ipap_value_field actFvalue = fieldAct.parse((fieldListIter->value[0]).getValue());
			data.insert_field(fItem.eno, fItem.ftype, actFvalue);		
		}
	}
		
	mes->include_data(templateId, data);

#ifdef DEBUG
    log->dlog(ch, "Ending addDataRecord");
#endif

}


void MAPIBiddingObjectParser::addOptionRecord(fieldDefList_t *fieldDefs, BiddingObject * biddingObjectPtr, 
									string recordId, time_t start, time_t stop, 
									fieldList_t &fieldList, uint16_t templateId, 
									ipap_message *mes )
{

#ifdef DEBUG
    log->dlog(ch, "Starting addOptionRecord");
#endif
	
	char *valueChr=NULL;
	ipap_data_record data(templateId);

	// Add the auction Id
	ipap_field idAuctionF = mes->get_field_definition( 0, IPAP_FT_IDAUCTION );
	valueChr = strdup((biddingObjectPtr->getAuctionIpAPId()).c_str());
	ipap_value_field fvalue3 = idAuctionF.get_ipap_value_field( valueChr, 
										( biddingObjectPtr->getAuctionIpAPId()).size() );
	free(valueChr);
	data.insert_field(0, IPAP_FT_IDAUCTION, fvalue3);
	
	// Add the BiddingObject Id
	ipap_field idBidF = mes->get_field_definition( 0, IPAP_FT_IDBIDDINGOBJECT );
	valueChr = strdup((biddingObjectPtr->getIpApId(getDomain())).c_str());
	ipap_value_field fvalue1 = idBidF.get_ipap_value_field( valueChr, 
										(biddingObjectPtr->getIpApId(getDomain())).size() );
	free(valueChr);									
	data.insert_field(0, IPAP_FT_IDBIDDINGOBJECT, fvalue1);

	// Add the Record Id
	ipap_field idRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	valueChr = strdup(recordId.c_str());
	ipap_value_field fvalue2 = idRecordIdF.get_ipap_value_field( valueChr, recordId.size() );
	free(valueChr);
	data.insert_field(0, IPAP_FT_IDRECORD, fvalue2);	

	//Add start y stop information which are mandatory.

	// Add the start time.
	assert (sizeof(uint64_t) >= sizeof(time_t));
	time_t time = start;
	uint64_t timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_field idStartF = mes->get_field_definition( 0, IPAP_FT_STARTSECONDS );
	ipap_value_field fvalueStart = idStartF.get_ipap_value_field( timeUint64 );
	data.insert_field(0, IPAP_FT_STARTSECONDS, fvalueStart);

	// Add the end time.
	ipap_field idStopF = mes->get_field_definition( 0, IPAP_FT_ENDSECONDS );
	time = stop;
	timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_value_field fvalueStop = idStopF.get_ipap_value_field( timeUint64 );
	data.insert_field(0, IPAP_FT_ENDSECONDS, fvalueStop);
	
	// Find the option template for this object type
	ipap_templ_type_t tempType = ipap_template::getTemplateType(biddingObjectPtr->getType(), IPAP_OPTIONS);
	
	// Only add other fields not include as mandatory fields.
	set<ipap_field_key> optionFields = ipap_template::getTemplateTypeMandatoryFields(tempType);
	set<ipap_field_key>::iterator iter;
	
	// Add the rest of the field within the record.
	fieldListIter_t fieldListIter;
	for (fieldListIter = fieldList.begin(); fieldListIter != fieldList.end(); ++fieldListIter)
	{
		// Search the field in the list of fields, previously it was verified its existance.
		fieldDefItem_t fItem = findField(fieldDefs, fieldListIter->name);
		
		iter = optionFields.find(ipap_field_key(fItem.eno, fItem.ftype));
		if (iter == optionFields.end()){
#ifdef DEBUG
			log->dlog(ch, "get_ipap_message - it is going to add %s eno:%d ftype:%d", 
					fieldListIter->name.c_str(), fItem.eno, fItem.ftype);
#endif
			
			ipap_field fieldAct = mes->get_field_definition(fItem.eno, fItem.ftype);
		
			//TODO AM: we have to fix for managing more than one value.
			ipap_value_field actFvalue = fieldAct.parse((fieldListIter->value[0]).getValue());
			data.insert_field(fItem.eno, fItem.ftype, actFvalue);		
		}
	}
		
	mes->include_data(templateId, data);

#ifdef DEBUG
    log->dlog(ch, "Ending addOptionRecord");
#endif

}


auction::fieldList_t 
MAPIBiddingObjectParser::readRecord( ipap_template *templ, fieldDefList_t *fieldDefs,
									 fieldValList_t *fieldVals, ipap_data_record &record,
									 string &auctionSet, string &auctionName,
									 string &bidSet, string &bidName, 
									 string &stype, string &status, string &recordId )
{

#ifdef DEBUG
    log->dlog(ch, "Starting readRecord");
#endif

	string recordName, bidId, auctionId;
	fieldList_t fields;
	fieldDataListIter_t fieldIter;
	for (fieldIter=record.begin(); fieldIter!=record.end(); ++fieldIter){
		ipap_field_key kField = fieldIter->first;
		ipap_value_field dFieldValue = fieldIter->second;
		
		fieldDefItem_t fItem = findField(fieldDefs, kField.get_eno() , kField.get_ftype());
		if ((fItem.name).empty()){
			ostringstream s;
			s << "BiddingObject Message Parser: Field eno:" << kField.get_eno();
			s << "fType:" << kField.get_ftype() << "is not parametrized";
			throw Error(s.str()); 
		} else {
			
			ipap_field field = templ->get_field( kField.get_eno(), kField.get_ftype() );

			// Search the record name field.
			if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDBIDDINGOBJECT)){
				// Parse the BiddingObject name if has not done yet.
				if (bidName.empty()){
					bidId = field.writeValue(dFieldValue);
					parseName(bidId, bidSet, bidName);
				}
			}
			if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDAUCTION)){
				// Parse the BiddingObject name if has not done yet.
				if (auctionName.empty()){
					auctionId = field.writeValue(dFieldValue);
					parseName(auctionId, auctionSet, auctionName);
				}
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDRECORD)){
				recordId = field.writeValue(dFieldValue);
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_BIDDINGOBJECTTYPE)){
				if (stype.empty()){	  
					stype = field.writeValue(dFieldValue);
				}
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_AUCTIONINGOBJECTSTATUS)){
				if (status.empty()){	  
					status = field.writeValue(dFieldValue);
				}
			}
			else {
				field_t item;
				item.name = fItem.name;
				item.type = fItem.type;
				item.len = fItem.len;				
				string value = field.writeValue(dFieldValue);
				parseFieldValue(fieldVals, value, &item);
				fields.push_back(item);
			}
		}
	}
	
#ifdef DEBUG
    log->dlog(ch, "Ending readRecord");
#endif
	
	return fields;
}


void 
MAPIBiddingObjectParser::verifyInsertTemplates(ipap_template *templData, ipap_template *templOption, 
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
		throw Error("MAPI BiddingObject Parser: Data Template %d given is not stored", templData->get_template_id()); 
	}
	
	try {
		templOption2 = templatesOut->get_template(templOption->get_template_id());
	} catch (Error &e){
		throw Error("MAPI BiddingObject Parser: Data Option Template %d given is not stored", templOption->get_template_id()); 
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

bool 
MAPIBiddingObjectParser::isTemplateSubtype(ipap_templ_subtype_t sybtype, ipap_templ_type_t type)
{
	bool val_return = false;
	
	// Verify whether or not the template is a data template
	set<ipap_templ_type_t> setTempl = ipap_template::getTemplates(sybtype);		
	set<ipap_templ_type_t>::iterator it = setTempl.find(type);
	if (it != setTempl.end())
		val_return =  true;
	
	return val_return;
}

ipap_template * 
MAPIBiddingObjectParser::findTemplate(ipap_template_container *templatesOut, 
									  uint16_t templId)
{

	try {
		ipap_template *templ = templatesOut->get_template(templId);
						
		return templ;
		
	} catch (Error &e) {
		log->elog(ch, e.getError().c_str());
		throw Error("MAPI BiddingObject Parser: missing template  %d", templId); 
	}

}



void 
MAPIBiddingObjectParser::parseAuctionKey(fieldDefList_t *fields, 
										 fieldValList_t *fieldVals,
										 const anslp::msg::xml_object_key &key,
										 biddingObjectDB_t *bids,
										 ipap_template_container *templates )
{

#ifdef DEBUG
    log->dlog(ch, "Starting parseAuctionKey %s", (key.to_string()).c_str());
#endif

    string auctionSet, auctionName;
    string bidSet, bidName, stype, status;
    elementList_t elements;
    optionList_t options;
	ipap_template *templData = NULL;
	ipap_template *templOption = NULL;
	BiddingObject *b = NULL;
	
    ipap_object_type_t type;

    try {

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
					
					
									
					set<ipap_templ_type_t> setOpt = ipap_template::getTemplates(IPAP_OPTIONS);
					set<ipap_templ_type_t>::iterator it2 = setOpt.find(templ->get_type());
					if (it2 != setOpt.end()){
						templOption = templ;
					}

				}
			}
		}

		// Verifies templates
		verifyInsertTemplates(templData, templOption, templates);

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
					ipap_template * templ = findTemplate(templates, templId );
												
					string elementName;
					fieldList_t readFields = readRecord(templ, fields, fieldVals, *dataRecordIter, 
														 auctionSet, auctionName, bidSet, 
															bidName, stype, status, elementName );
					type = parseType(stype);
					
					if (isTemplateSubtype(IPAP_RECORD, templ->get_type())){
						elements[elementName] = readFields;
						++NbrDataRead;
					}
					
					if (isTemplateSubtype(IPAP_OPTIONS, templ->get_type())){	
						options.push_back(pair<string, fieldList_t>(elementName, readFields));
						NbrOptionRead++;
					}
				}
				
				if (NbrDataRead == 0){
					throw Error("MAPI BiddingObject Parser: a data template was not given %s", (key.to_string()).c_str()); 
				}
				
				if (NbrOptionRead == 0){
					throw Error("MAPI BiddingObject Parser: an option template was not given %s", (key.to_string()).c_str()); 
				}
				// data has been read, so we can finish.
				break;
			}
		}    
     
        b = new BiddingObject(auctionSet, auctionName, bidSet, bidName, type, elements, options);
        
		int istatus = ParserFcts::parseInt(status, 0, 16);
		b->setState((AuctioningObjectState_t) istatus);        
		
        bids->push_back(b);

#ifdef DEBUG
		// debug info
		log->dlog(ch, "BiddingObject %s.%s - %s", bidSet.c_str(), bidName.c_str(), (b->getInfo()).c_str());
#endif
            
    } catch (Error &e) {
		if (b != NULL) {
			saveDelete(b);
	    }
		log->elog(ch, e);
		throw e;
    }
}


void 
MAPIBiddingObjectParser::parse(fieldDefList_t *fieldDefs, 
							   fieldValList_t *fieldVals,
							   ipap_message *message, 
							   biddingObjectDB_t *bids,
							   ipap_template_container *templates )
{
	try
	{

#ifdef DEBUG
		log->dlog(ch, "Starting parse");
#endif

		anslp::msg::anslp_ipap_message mes(*message);
				
		split(const_cast< anslp::msg::anslp_ipap_message &>(mes));
		
		anslp::msg::xmlDataRecordIterList_t iter;
		for (iter = objectDataRecords.begin(); iter != objectDataRecords.end();  ++iter)
		{
			parseAuctionKey(fieldDefs, fieldVals, iter->first, bids, templates); 
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
void 
MAPIBiddingObjectParser::get_ipap_message(fieldDefList_t *fieldDefs, 
										  BiddingObject *biddingObjectPtr, 
										  Auction *auctionPtr, 
										  ipap_template_container *templates,
										  ipap_message *message)
{
#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message");
#endif	

	time_t now = time(NULL);
	ipap_templ_type_t tempType;

	// Verifies that the BiddingObject is for the auction given.
	assert((biddingObjectPtr->getAuctionSet() == auctionPtr->getSetName()) && 
			(biddingObjectPtr->getAuctionName() == auctionPtr->getAuctionName() ));

	uint16_t dataTemplateId, optionTemplateId;
		
	// Find both templates types for the bidding object.
	tempType = ipap_template::getTemplateType(biddingObjectPtr->getType(), IPAP_RECORD);
	dataTemplateId = auctionPtr->getBiddingObjectTemplate(biddingObjectPtr->getType(), tempType);
								
	tempType = ipap_template::getTemplateType(biddingObjectPtr->getType(), IPAP_OPTIONS);
	optionTemplateId = auctionPtr->getBiddingObjectTemplate(biddingObjectPtr->getType(),tempType);
	
	// Insert BiddingObject's templates.
	ipap_template *bidTempl = templates->get_template(dataTemplateId);
	message->make_template(bidTempl);

#ifdef DEBUG
    log->dlog(ch, "Finish inserting data template - NumFields:%d", bidTempl->get_numfields());
#endif

	ipap_template *optTempl = templates->get_template(optionTemplateId);
	message->make_template(optTempl);
	
#ifdef DEBUG
    log->dlog(ch, "Finish inserting option template - NumFields:%d", optTempl->get_numfields());
#endif		

	// Include data records.
	elementListIter_t elemIter;
	for ( elemIter = biddingObjectPtr->getElements()->begin(); elemIter != biddingObjectPtr->getElements()->end(); ++elemIter)
	{
		addDataRecord(fieldDefs, biddingObjectPtr, elemIter->first, elemIter->second, 
				  dataTemplateId, message );
	}
	
	biddingObjectIntervalList_t *intervalList = new biddingObjectIntervalList_t();
	biddingObjectPtr->calculateIntervals(now, intervalList);
	
	// Include option records.
	int i = 0;
	optionListIter_t optIter;
	for ( optIter = biddingObjectPtr->getOptions()->begin(); optIter != biddingObjectPtr->getOptions()->end(); ++optIter)
	{
		
		biddingObjectInterval_t bidInterval = ((*intervalList)[i]).second;
		addOptionRecord(fieldDefs, biddingObjectPtr, optIter->first, bidInterval.start, 
						bidInterval.stop, optIter->second, optionTemplateId, message );
		i = i + 1;
	}
	
	saveDelete(intervalList);
	
#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_message");
#endif	
	
}											

ipap_message * 
MAPIBiddingObjectParser::get_ipap_message(fieldDefList_t *fieldDefs, 
										  BiddingObject *biddingObject, 
										  Auction *auction, 
										  ipap_template_container *templates )
{

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_messages");
#endif	
	
	ipap_message *mes = new ipap_message(getDomain(), IPAP_VERSION, true);
			
	if ( (biddingObject->getAuctionSet() == auction->getSetName()) && 
		   (biddingObject->getAuctionName() == auction->getAuctionName()) ){
		get_ipap_message(fieldDefs, biddingObject, auction, templates, mes);
		mes->output();
	} else {
		throw Error("the auction is not the same as the one referenced in the bidding object");
	}
	

#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_messages");
#endif
	
	return mes;
		
}
