
/*!  \file   MAPIBidParser.cpp

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
    parser for API message bid syntax

    $Id: MAPIBidParser.cpp 2015-07-24 15:14:00 amarentes $
*/

#include "ParserFcts.h"
#include "Constants.h"
#include "MAPIBidParser.h"
#include "Timeval.h"

using namespace auction;

MAPIBidParser::MAPIBidParser()
    : IpApMessageParser()
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIBidParser" );
    
#ifdef DEBUG
    log->dlog(ch, "Constructor MAPIBidParser");
#endif    
}

												

void MAPIBidParser::addDataRecord(fieldDefList_t *fieldDefs, string auctionId, string bidId, string recordId, 
							  fieldList_t &fieldList, uint16_t templateId, ipap_message *mes )
{

#ifdef DEBUG
    log->dlog(ch, "Starting addDataRecord");
#endif

	ipap_data_record data(templateId);

	// Add the auction Id
	ipap_field idAuctionF = mes->get_field_definition( 0, IPAP_FT_IDAUCTION );
	ipap_value_field fvalue3 = idAuctionF.get_ipap_value_field( 
								strdup(auctionId.c_str()), auctionId.size() );
	data.insert_field(0, IPAP_FT_IDAUCTION, fvalue3);
	
	// Add the Bid Id
	ipap_field idBidF = mes->get_field_definition( 0, IPAP_FT_IDBID );
	ipap_value_field fvalue1 = idBidF.get_ipap_value_field( 
									strdup(bidId.c_str()), bidId.size() );
	data.insert_field(0, IPAP_FT_IDBID, fvalue1);

	// Add the Record Id
	ipap_field idRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	ipap_value_field fvalue2 = idRecordIdF.get_ipap_value_field( 
									strdup(recordId.c_str()), recordId.size() );
	data.insert_field(0, IPAP_FT_IDRECORD, fvalue2);	

	// Add the rest of the field within the record.
	fieldListIter_t fieldListIter;
	for (fieldListIter = fieldList.begin(); fieldListIter != fieldList.end(); ++fieldListIter)
	{
		// Search the field in the list of fields, previously it was verified its existance.
		fieldDefItem_t fItem = findField(fieldDefs, fieldListIter->name);

#ifdef DEBUG
		log->dlog(ch, "get_ipap_message - it is going to add %s eno:%d ftype:%d", 
					fieldListIter->name.c_str(), fItem.eno, fItem.ftype);
#endif
		ipap_field fieldAct = mes->get_field_definition(fItem.eno, fItem.ftype);
		
		//TODO AM: we have to fix to manage more than one value.
		ipap_value_field actFvalue = fieldAct.parse((fieldListIter->value[0]).getValue());
		data.insert_field(fItem.eno, fItem.ftype, actFvalue);		
	}
		
	mes->include_data(templateId, data);

#ifdef DEBUG
    log->dlog(ch, "Ending addDataRecord");
#endif

}


void MAPIBidParser::addOptionRecord(fieldDefList_t *fieldDefs, string auctionId, 
									string bidId, string recordId, time_t start, time_t stop, 
									fieldList_t &fieldList, uint16_t templateId, 
									ipap_message *mes )
{

#ifdef DEBUG
    log->dlog(ch, "Starting addOptionRecord");
#endif

	ipap_data_record data(templateId);

	// Add the auction Id
	ipap_field idAuctionF = mes->get_field_definition( 0, IPAP_FT_IDAUCTION );
	ipap_value_field fvalue3 = idAuctionF.get_ipap_value_field( 
								strdup(auctionId.c_str()), auctionId.size() );
	data.insert_field(0, IPAP_FT_IDAUCTION, fvalue3);
	
	// Add the Bid Id
	ipap_field idBidF = mes->get_field_definition( 0, IPAP_FT_IDBID );
	ipap_value_field fvalue1 = idBidF.get_ipap_value_field( 
									strdup(bidId.c_str()), bidId.size() );
	data.insert_field(0, IPAP_FT_IDBID, fvalue1);

	// Add the Record Id
	ipap_field idRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	ipap_value_field fvalue2 = idRecordIdF.get_ipap_value_field( 
									strdup(recordId.c_str()), recordId.size() );
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
	
	// Only add other fields not include as mandatory fields.
	set<ipap_field_key> optionFields = ipap_template::getTemplateTypeMandatoryFields(IPAP_OPTNS_BID_TEMPLATE);
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
		
			//TODO AM: we have to fix to manage more than one value.
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
MAPIBidParser::readBidRecord( ipap_template *templ, 
							  fieldDefList_t *fieldDefs,
							  fieldValList_t *fieldVals,
							  ipap_data_record &record,
							  string &auctionSet, string &auctionName,
							  string &bidSet, string &bidName,
							  string &recordId )
{

#ifdef DEBUG
    log->dlog(ch, "Starting readBidRecord");
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
			s << "Bid Message Parser: Field eno:" << kField.get_eno();
			s << "fType:" << kField.get_ftype() << "is not parametrized";
			throw Error(s.str()); 
		} else {
			
			ipap_field field = templ->get_field( kField.get_eno(), kField.get_ftype() );

			// Search the record name field.
			if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDBID)){
				// Parse the bid name if has not done yet.
				if (bidName.empty()){
					bidId = field.writeValue(dFieldValue);
					parseName(bidId, bidSet, bidName);
				}
			}
			if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDAUCTION)){
				// Parse the bid name if has not done yet.
				if (auctionName.empty()){
					auctionId = field.writeValue(dFieldValue);
					parseName(auctionId, auctionSet, auctionName);
				}
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDRECORD)){
				recordId = field.writeValue(dFieldValue);
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
    log->dlog(ch, "Ending readBidRecord");
#endif
	
	return fields;
}


void 
MAPIBidParser::verifyInsertTemplates(ipap_template *templData, ipap_template *templOption, 
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
		throw Error("MAPI Bid Parser: Data Template %d given is not stored", templData->get_template_id()); 
	}
	
	try {
		templOption2 = templatesOut->get_template(templOption->get_template_id());
	} catch (Error &e){
		throw Error("MAPI Bid Parser: Data Option Template %d given is not stored", templOption->get_template_id()); 
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
MAPIBidParser::findTemplate(ipap_template *templData, ipap_template *templOption,
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
		if (templ->get_type() == IPAP_SETID_BID_TEMPLATE){
			templData = templ;
		}
						
		if  (templ->get_type() == IPAP_OPTNS_BID_TEMPLATE){
			templOption = templ;
		}
		
		return templ;
		
	} catch (Error &e) {
		log->elog(ch, e.getError().c_str());
		throw Error("MAPI Bid Parser: missing template  %d", templId); 
	}

}



void MAPIBidParser::parseAuctionKey(fieldDefList_t *fields, 
									fieldValList_t *fieldVals,
									const anslp::msg::xml_object_key &key,
									bidDB_t *bids,
									ipap_template_container *templates )
{

#ifdef DEBUG
    log->dlog(ch, "Starting parseAuctionKey %s", (key.to_string()).c_str());
#endif

    string auctionSet, auctionName;
    string bidSet, bidName;
    elementList_t elements;
    optionList_t options;
	ipap_template *templData = NULL;
	ipap_template *templOption = NULL;
	Bid *b = NULL;
    

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
					
					if (templ->get_type() == IPAP_SETID_BID_TEMPLATE){
						templData = templ;
					}
						
					if  (templ->get_type() == IPAP_OPTNS_BID_TEMPLATE){
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
					ipap_template *templ = findTemplate(templData, templOption, templates, templId );
					
					// Read a data record for a data template 
					if (templData != NULL){
						if (templId == templData->get_template_id()){
							
							string elementName;
							fieldList_t elemtFields = readBidRecord(templData, fields, fieldVals, *dataRecordIter, 
											auctionSet, auctionName, bidSet, bidName, elementName );
											
							elements[elementName] = elemtFields;
							
							++NbrDataRead;
						}
					}
					
					// Read a data record for an option template 
					if (templOption != NULL){
						if (templId == templOption->get_template_id()){
							
							string optionId;
							fieldList_t optionFields = 
											readBidRecord(templOption, fields, fieldVals, *dataRecordIter, 
														auctionSet, auctionName, bidSet, bidName, optionId );
											
							options.push_back(pair<string, fieldList_t>(optionId, optionFields));
							
							NbrOptionRead++;
						}
					}
				}
				
				if (NbrDataRead == 0){
					throw Error("MAPI Bid Parser: a data template was not given"); 
				}
				
				if (NbrOptionRead == 0){
					throw Error("MAPI Bid Parser: an option template was not given %s", (key.to_string()).c_str()); 
				}
				// data has been read, so we can finish.
				break;
			}
		}    
     
        b = new Bid(auctionSet, auctionName, bidSet, bidName, elements, options);
        bids->push_back(b);

#ifdef DEBUG
		// debug info
		log->dlog(ch, "bid %s.%s - %s", bidSet.c_str(), bidName.c_str(), (b->getInfo()).c_str());
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
MAPIBidParser::parse(fieldDefList_t *fieldDefs, fieldValList_t *fieldVals,
					 ipap_message *message, bidDB_t *bids,
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
			if ((iter->first).get_object_type() == anslp::msg::IPAP_BID){
				parseAuctionKey(fieldDefs, fieldVals, iter->first, bids, templates); 
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
void 
MAPIBidParser::get_ipap_message(fieldDefList_t *fieldDefs, Bid *bidPtr, Auction *auctionPtr, 
								ipap_template_container *templates,
								int domainId, ipap_message *message)
{
#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message");
#endif	

	time_t now = time(NULL);

	// Verifies that the bid is for the auction given.
	assert((bidPtr->getAuctionSet() == auctionPtr->getSetName()) && 
			(bidPtr->getAuctionName() == auctionPtr->getAuctionName() ));

	// We assume that both templates were already sent to the other part.
	uint16_t dataTemplateId, optionTemplateId;
	
	dataTemplateId = auctionPtr->getDataBidTemplate();
	optionTemplateId = auctionPtr->getOptionBidTemplate();
	
	// Insert bid's templates.
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

	// Set auction Id.
	string idAuctionS = bidPtr->getAuctionSet() + "." + bidPtr->getAuctionName();


	// Set bid Id.
	string idBidS;
	if ((bidPtr->getBidSet()).empty()){
		ostringstream ssA;
		ssA << "Agent_" << domainId;
		idBidS =  ssA.str() + "." + bidPtr->getBidName();
	} else {
		idBidS = bidPtr->getBidSet() + "." + bidPtr->getBidName();
	}

	// Include data records.
	elementListIter_t elemIter;
	for ( elemIter = bidPtr->getElements()->begin(); elemIter != bidPtr->getElements()->end(); ++elemIter)
	{
		addDataRecord(fieldDefs, idAuctionS, idBidS, elemIter->first, elemIter->second, 
				  dataTemplateId, message );
	}
	
	bidIntervalList_t *intervalList = new bidIntervalList_t();
	bidPtr->calculateIntervals(now, intervalList);
	
	// Include option records.
	int i = 0;
	optionListIter_t optIter;
	for ( optIter = bidPtr->getOptions()->begin(); optIter != bidPtr->getOptions()->end(); ++optIter)
	{
		
		bidInterval_t bidInterval = ((*intervalList)[i]).second;
		addOptionRecord(fieldDefs, idAuctionS, idBidS, optIter->first, bidInterval.start, 
						bidInterval.stop, optIter->second, optionTemplateId, message );
		i = i + 1;
	}
	

	
	
#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_message");
#endif	
	
}											

ipap_message * 
MAPIBidParser::get_ipap_message(fieldDefList_t *fieldDefs, 
								 bidDB_t *bids, auctionDB_t *auctions, 
								 ipap_template_container *templates, int domainId )
{

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_messages");
#endif	
	
	ipap_message *mes = new ipap_message(domainId, IPAP_VERSION, true);
	
	bidDBIter_t bidIter;
	for (bidIter=bids->begin(); bidIter!=bids->end(); ++bidIter)
	{
		Bid *b = *bidIter;
		
#ifdef DEBUG
		log->dlog(ch, "Bid to include %s.%s belonging to auction:%s.%s", 
				(b->getBidSet()).c_str(), b->getBidName().c_str(), 
				(b->getAuctionSet()).c_str(), b->getAuctionName().c_str() );
#endif			
		auctionDBIter_t auctionIter;
		for (auctionIter = auctions->begin(); auctionIter != auctions->end(); ++auctionIter){
			Auction *a = *auctionIter;

#ifdef DEBUG
			log->dlog(ch, "Auction %s.%s", (a->getSetName()).c_str(), a->getAuctionName().c_str() );
#endif
			
			if ( (b->getAuctionSet() == a->getSetName()) && (b->getAuctionName() == a->getAuctionName()) ){
				get_ipap_message(fieldDefs, b, a, templates, domainId, mes);
				break;
			}
		}
	}
	mes->output();

#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_messages");
#endif
	
	return mes;
		
}
