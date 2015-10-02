
/*!  \file   MAPIAllocationParser.cpp

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
    parser for API text bid syntax

    $Id: MAPIAllocationParser.cpp 2015-09-30 8:00:00 amarentes $
*/

#include "ParserFcts.h"
#include "Constants.h"
#include "MAPIAllocationParser.h"
#include "Timeval.h"

using namespace auction;

MAPIAllocationParser::MAPIAllocationParser()
    : MAPIIpApMessageParser()
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIAllocationParser" );
}


uint16_t 
MAPIAllocationParser::addFieldsRecordTemplate(fieldDefList_t *fieldDefs, 
											  Allocation *allocationPtr, 
											  ipap_message *mes)
{

#ifdef DEBUG
    log->dlog(ch, "Starting addFieldsRecordTemplate");
#endif
	
	int nfields = 0;
	uint16_t templateId;
	map<string,fieldDefItem_t> fields;
	
	// Loop through the elements in order to put them in a data record
	fieldListIter_t fieldListIter;
	for (fieldListIter = allocationPtr->getFields()->begin(); 
			fieldListIter != allocationPtr->getFields()->end(); ++fieldListIter){
		
		field_t field = *fieldListIter;
		
#ifdef DEBUG
		log->dlog(ch, "Field data: %s", field.getInfo().c_str());
#endif		
		fieldDefItem_t fItem = findField(fieldDefs, field.name);
		if ((fItem.name).empty()){
			ostringstream s;
			s << "Allocation Message Parser: Field name:" << field.name
			  << "is not parametrized";
			throw Error(s.str()); 
		}
		else{
			fields[field.name] = fItem;
		}
	}

	// Loop through the fields in order to include them in the template
	// we include additionally IDAUCTION, IDBID, IDALLOCATION, and IDRECORD.
	nfields = fields.size() + 4; 
	templateId = mes->new_data_template( nfields, IPAP_SETID_ALLOCATION_TEMPLATE );
	
	//put the AUCTION ID
	mes->add_field(templateId, 0, IPAP_FT_IDAUCTION);
	//put the BID ID
	mes->add_field(templateId, 0, IPAP_FT_IDBID);
	//put the ALLOCATION ID
	mes->add_field(templateId, 0, IPAP_FT_IDALLOCATION);
	// put the RECORD ID.
	mes->add_field(templateId, 0, IPAP_FT_IDRECORD);
	
	// Loop thought the fields to include and include them.
	map<string,fieldDefItem_t>::iterator fieldIter;
	for (fieldIter = fields.begin(); fieldIter != fields.end(); ++fieldIter){
		mes->add_field(templateId, fieldIter->second.eno, fieldIter->second.ftype);
	}	

#ifdef DEBUG
    log->dlog(ch, "Ending addFieldsRecordTemplate");
#endif
	
	return templateId;
}

uint16_t 
MAPIAllocationParser::addFieldsOptionTemplate(fieldDefList_t *fieldDefs, 
											  Allocation *allocationPtr, 
											  ipap_message *mes)
{

#ifdef DEBUG
    log->dlog(ch, "Starting addFieldsOptionTemplate");
#endif
	
	int nfields = 0;
	uint16_t templateId;

	// we include IDAUCTION, IDBID, IDALLOCATION, STARTSECONDS, ENDSECONDS, and IDRECORD.
	nfields = 6;
	templateId = mes->new_data_template( nfields, IPAP_OPTNS_ALLOCATION_TEMPLATE );
	
	//put the AUCTION ID
	mes->add_field(templateId, 0, IPAP_FT_IDAUCTION);
	//put the BID ID
	mes->add_field(templateId, 0, IPAP_FT_IDBID);
	//put the ALLOCATION ID
	mes->add_field(templateId, 0, IPAP_FT_IDALLOCATION);
	//put the STARTSECONDS
	mes->add_field(templateId, 0, IPAP_FT_STARTSECONDS);
	//put the ENDSECONDS
	mes->add_field(templateId, 0, IPAP_FT_ENDSECONDS);
	// put the RECORD ID.
	mes->add_field(templateId, 0, IPAP_FT_IDRECORD);

#ifdef DEBUG
    log->dlog(ch, "Ending addFieldsOptionTemplate");
#endif
		
	return templateId;
}


void MAPIAllocationParser::addRecord(string auctionId, string bidId, string allocationId,
									 fieldList_t * fields, fieldDefList_t *fieldDefs,
									 uint16_t templateId, ipap_message *mes )
{

#ifdef DEBUG
    log->dlog(ch, "Starting addRecord templateId:%d", templateId);
#endif
	
	ipap_data_record dataElement(templateId);
	
	// Add the auctionId
	ipap_field idAuctionF = mes->get_field_definition( 0, IPAP_FT_IDAUCTION );
	ipap_value_field fvalue1 = idAuctionF.get_ipap_value_field( 
									strdup(auctionId.c_str()), auctionId.size() );
	dataElement.insert_field(0, IPAP_FT_IDAUCTION, fvalue1);
	
	// Add the bidId
	ipap_field idBidF = mes->get_field_definition( 0, IPAP_FT_IDBID );
	ipap_value_field fvalue2 = idBidF.get_ipap_value_field( 
									strdup(bidId.c_str()), bidId.size() );
	dataElement.insert_field(0, IPAP_FT_IDBID, fvalue2);

	// Add the AllocationId
	ipap_field idAllocationF = mes->get_field_definition( 0, IPAP_FT_IDALLOCATION );
	ipap_value_field fvalue3 = idAllocationF.get_ipap_value_field( 
									strdup(allocationId.c_str()), allocationId.size() );
	dataElement.insert_field(0, IPAP_FT_IDALLOCATION, fvalue3);

	// Add a fixed Record Id
	string recordId = "Record_1";
	ipap_field idRecordF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	ipap_value_field fvalue4 = idRecordF.get_ipap_value_field( 
									strdup(recordId.c_str()), recordId.size() );
	dataElement.insert_field(0, IPAP_FT_IDRECORD, fvalue4);
	
	// Add the rest of the fields.
	fieldListIter_t fieldListIter;
	for (fieldListIter = fields->begin(); fieldListIter != fields->end(); ++fieldListIter)
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
		dataElement.insert_field(fItem.eno, fItem.ftype, actFvalue);		
	}
	
	mes->include_data(templateId, dataElement);


#ifdef DEBUG
    log->dlog(ch, "Ending addRecord");
#endif	
}


void 
MAPIAllocationParser::addOptionRecord(string auctionId, 
									  string bidId,
									  string allocationId,
									  string recordId,
									  alloc_interval_t allocInt, 
									  uint16_t templateId, 
									  ipap_message *mes )
{

#ifdef DEBUG
    log->dlog(ch, "Starting addOptionRecord templateId:%d", templateId);
#endif

	ipap_data_record dataOption(templateId);

	// Add the Auction ID
	ipap_field idAuctionF = mes->get_field_definition( 0, IPAP_FT_IDAUCTION );
	ipap_value_field fvalue1 = idAuctionF.get_ipap_value_field( 
									strdup(auctionId.c_str()), auctionId.size() );
	dataOption.insert_field(0, IPAP_FT_IDAUCTION, fvalue1);

	
	// Add the Bid ID
	ipap_field idBidF = mes->get_field_definition( 0, IPAP_FT_IDBID );
	ipap_value_field fvalue2 = idBidF.get_ipap_value_field( 
									strdup(bidId.c_str()), bidId.size() );
	dataOption.insert_field(0, IPAP_FT_IDBID, fvalue2);

	// Add the Allocation ID
	ipap_field idAllocationF = mes->get_field_definition( 0, IPAP_FT_IDALLOCATION );
	ipap_value_field fvalue3 = idAllocationF.get_ipap_value_field( 
									strdup(allocationId.c_str()), allocationId.size() );
	dataOption.insert_field(0, IPAP_FT_IDALLOCATION, fvalue3);
	
	// Add the Record Id
	ipap_field idRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	ipap_value_field fvalue4 = idRecordIdF.get_ipap_value_field( 
									strdup(recordId.c_str()), recordId.size() );
	dataOption.insert_field(0, IPAP_FT_IDRECORD, fvalue4);
		
	// Add the start datetime
	assert (sizeof(uint64_t) >= sizeof(time_t));
	time_t time = allocInt.start;
	uint64_t timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_field idStartF = mes->get_field_definition( 0, IPAP_FT_STARTSECONDS );
	ipap_value_field fvalue5 = idStartF.get_ipap_value_field( timeUint64 );
	dataOption.insert_field(0, IPAP_FT_STARTSECONDS, fvalue5);
	
	// Add the endtime
	ipap_field idStopF = mes->get_field_definition( 0, IPAP_FT_ENDSECONDS );
	time = allocInt.stop;
	timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_value_field fvalue6 = idStopF.get_ipap_value_field( timeUint64 );
	dataOption.insert_field(0, IPAP_FT_ENDSECONDS, fvalue6);
			
	mes->include_data(templateId, dataOption);

#ifdef DEBUG
    log->dlog(ch, "Ending addOptionRecord");
#endif
	
}


auction::fieldList_t 
MAPIAllocationParser::readAllocationData( ipap_template *templ, 
										  fieldDefList_t *fieldDefs,
										  fieldValList_t *fieldVals,
										  ipap_data_record &record,
										  string &auctionSet,
										  string &auctionName,
										  string &bidSet,
										  string &bidName,
										  string &allocationSet,
										  string &allocationName )
{

#ifdef DEBUG
    log->dlog(ch, "Starting readAllocationData");
#endif

	string recordName, auctionId, bidId, allocationId;
	fieldList_t fields;

	fieldDataListIter_t fieldIter;
	for (fieldIter=record.begin(); fieldIter!=record.end(); ++fieldIter){
		ipap_field_key kField = fieldIter->first;
		ipap_value_field dFieldValue = fieldIter->second;
		
		fieldDefItem_t fItem = findField(fieldDefs, kField.get_eno() , kField.get_ftype());
		if ((fItem.name).empty()){
			ostringstream s;
			s << "Allocation Message Parser: Field eno:" << kField.get_eno();
			s << "fType:" << kField.get_ftype() << "is not parametrized";
			throw Error(s.str()); 
		}
		else{
			ipap_field field = templ->get_field( kField.get_eno(), kField.get_ftype() );

			// Search the Auction name field.
			if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDAUCTION)){
				// Parse the bid name if has not done yet.
				if (auctionName.empty()){
					auctionId = field.writeValue(dFieldValue);
					parseName(auctionId, auctionSet, auctionName);
				}
			}
			// Search the Bid name field.
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDBID)){
				// Parse the bid name if has not done yet.
				if (bidName.empty()){
					bidId = field.writeValue(dFieldValue);
					parseName(bidId, bidSet, bidName);
				}
			}
			// Search the Allocation Id field.
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDALLOCATION)){
				// Parse the bid name if has not done yet.
				if ( allocationName.empty() ){
					allocationId = field.writeValue(dFieldValue);
					parseName(allocationId, allocationSet, allocationName);
				}
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDRECORD)){
				recordName = field.writeValue(dFieldValue);
			}
			else {
				field_t item;
				item.name = fItem.name;
				item.type = fItem.type;
				string value = field.writeValue(dFieldValue);
				parseFieldValue(fieldVals, value, &item);
				fields.push_back(item);
			}
		}
	}

#ifdef DEBUG
    log->dlog(ch, "Ending readAllocationData");
#endif
		
	return fields;
}

alloc_interval_t 
MAPIAllocationParser::readOptionData(ipap_template *templ, 
 								     fieldDefList_t *fieldDefs,
									 ipap_data_record &record)
{

#ifdef DEBUG
    log->dlog(ch, "Starting readOptionData");
#endif

	alloc_interval_t interval;

	fieldDataListIter_t fieldIter;
	for (fieldIter=record.begin(); fieldIter!=record.end(); ++fieldIter){
		ipap_field_key kField = fieldIter->first;
		ipap_value_field dFieldValue = fieldIter->second;
		
		fieldDefItem_t fItem = findField(fieldDefs, kField.get_eno() , kField.get_ftype());
		if ((fItem.name).empty()){
			ostringstream s;
			s << "Allocation Message Parser: Field eno:" << kField.get_eno();
			s << "fType:" << kField.get_ftype() << "is not parametrized";
			throw Error(s.str()); 
		}
		else{
			ipap_field field = templ->get_field( kField.get_eno(), kField.get_ftype() );

			// Search the auction name field.
			if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDAUCTION)){
				continue;
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDBID )){
				continue;
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDALLOCATION )){
				continue;
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDRECORD )){
				continue;
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_STARTSECONDS)){
				interval.start = (time_t) dFieldValue.get_value_int64();
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_ENDSECONDS)){
				interval.stop = (time_t) dFieldValue.get_value_int64();
			}
			else {
				ostringstream s;
				s << "Allocation Message Parser: Field eno:" << kField.get_eno();
				s << "fType:" << kField.get_ftype();
				s << " was not expected to be included ";
				throw Error(s.str()); 
			}
		}
	}

#ifdef DEBUG
    log->dlog(ch, "Ending readOptionData");
#endif

	return interval;
}


void MAPIAllocationParser::parse(fieldDefList_t *fields, 
						  fieldValList_t *fieldVals,
						  ipap_message *message,
						  allocationDB_t *allocations,
						  AllocationIdSource *idSource,
						  ipap_message *messageOut )
{
	
#ifdef DEBUG
    log->dlog(ch, "Starting parse");
#endif
	
    string auctionSet, auctionName, bidSet, bidName, allocationSet, allocationName;
    fieldList_t allocFields;
    allocationIntervalList_t allocIntervals;

    try {
		// Read the record template 
		ipap_template *templ = readTemplate(message, IPAP_SETID_ALLOCATION_TEMPLATE);
		
		// Read the option template
		ipap_template *templOpt = readTemplate(message, IPAP_OPTNS_ALLOCATION_TEMPLATE); 
			
		// Read the record data associated with the record template.
		if (templ != NULL){
			dataRecordList_t dRecordList = readDataRecords(message, templ->get_template_id());
			
			if (dRecordList.size() == 0){
				throw Error("Allocation Message Parser: a record data allocation  was not given"); 
			} 
			else if (dRecordList.size() > 1 ){
				throw Error("Allocation Message Parser: more than a record data allocation was given"); 
			}
			else {
				dateRecordListIter_t dataIter;
				for (dataIter = dRecordList.begin(); dataIter != dRecordList.end(); ++dataIter)
				{
					allocFields = readAllocationData(templ, fields, 
										fieldVals, *dataIter,auctionSet, 
										auctionName, bidSet, bidName, 
										allocationSet, allocationName );
				}
			}
		} else {
			throw Error("Allocation Message Parser: missing record template data"); 
		}
		
		
		// Read the option data record associated with the option data bid template.
		if (templOpt != NULL){
			
			dataRecordList_t dOptRecordList = readDataRecords(message, templOpt->get_template_id());
			if (dOptRecordList.size() == 0){
				throw Error("Allocation Message Parser: the option data \
								was not given for template:%d", 
									templOpt->get_template_id()); 
			} else {
				dateRecordListIter_t dataIter;
				for (dataIter = dOptRecordList.begin(); dataIter != dOptRecordList.end(); ++dataIter)
				{
					allocIntervals.push_back(readOptionData(templOpt, fields, *dataIter ));
				}
			}		
		} else{
			throw Error("Allocation Message Parser: missing option data template data"); 
		}
    
     
        Allocation *a = new Allocation(auctionSet, auctionName, bidSet, 
									  bidName, allocationSet, allocationName, 
									  allocFields, allocIntervals);
        allocations->push_back(a);

#ifdef DEBUG
		// debug info
		log->dlog(ch, "bid %s.%s - %s", bidName.c_str(), bidName.c_str(), (a->getInfo()).c_str());
#endif
            
    } catch (Error &e) {
       log->elog(ch, e);
       throw e;
    }
}


/* ------------------------- getMessage ------------------------- */
ipap_message * 
MAPIAllocationParser::get_ipap_message(Allocation *allocationPtr, 
									   fieldDefList_t *fieldDefs)
{
#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message");
#endif	

	// We assume that both templates were already sent to the other part.
	
	uint16_t optionTemplateId, recordTemplateId;
	ipap_message *mes = new ipap_message();
	
	// Add the option bid template
	optionTemplateId = addFieldsOptionTemplate(fieldDefs, allocationPtr, mes);

#ifdef DEBUG
    log->dlog(ch, "Option Template Id:%d", optionTemplateId);
#endif	
	
	// Loop through the fields in order to put them in a bid Template record
	recordTemplateId = addFieldsRecordTemplate(fieldDefs, allocationPtr, mes);

#ifdef DEBUG
    log->dlog(ch, "Record Template Id:%d", recordTemplateId);
#endif	
	
	// Build the Auction id
	string auctionId = allocationPtr->getAuctionSet() + "." + allocationPtr->getAuctionName();
	
	// Build the Id of the bid.
	string bidId = allocationPtr->getBidSet() + "." + allocationPtr->getBidName();

	// Build the Id of the allocation.
	string allocationId = allocationPtr->getAllocationSet() + "." 
							+ allocationPtr->getAllocationName();
			
	// Add fields in a data record
	addRecord(auctionId, bidId, allocationId, 
			   allocationPtr->getFields(), fieldDefs, recordTemplateId, mes );
	
	// Loop through the related intervals to include them in a option record.
	allocationIntervalListIter_t allocIntervalListIter;
	int recordId = 1;
	for (allocIntervalListIter = allocationPtr->getIntervals()->begin(); 
			allocIntervalListIter != allocationPtr->getIntervals()->end(); 
				++allocIntervalListIter){
					
		ostringstream ss;
		ss << "Record_" << recordId;
		string srecordId = ss.str();
		
		addOptionRecord(auctionId, bidId, allocationId, srecordId, 
							*allocIntervalListIter, optionTemplateId, mes);
		recordId = recordId + 1;
	}
	
#ifdef DEBUG
    log->dlog(ch, "get_ipap_message finished");
#endif	
	
	return mes;
}											

vector<ipap_message *> 
MAPIAllocationParser::get_ipap_messages(fieldDefList_t *fieldDefs, 
										allocationDB_t *allocations)
{

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_messages");
#endif	
	
	vector<ipap_message *> vct_return;
	allocationDBIter_t allocationIter;
	
	for (allocationIter=allocations->begin();
				allocationIter!=allocations->end(); ++allocationIter){
		Allocation *a = *allocationIter;
		ipap_message *mes =get_ipap_message(a, fieldDefs);
		vct_return.push_back(mes);
	}

#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_messages");
#endif
	
	return vct_return;
		
}
