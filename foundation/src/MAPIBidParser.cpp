
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
    parser for API text bid syntax

    $Id: MAPIBidParser.cpp 2015-07-24 15:14:00 amarentes $
*/

#include "ParserFcts.h"
#include "Constants.h"
#include "MAPIBidParser.h"
#include "Timeval.h"

using namespace auction;

MAPIBidParser::MAPIBidParser()
    : MAPIIpApMessageParser()
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIBidParser" );
    
#ifdef DEBUG
    log->dlog(ch, "Constructor MAPIBidParser");
#endif    
}


uint16_t 
MAPIBidParser::addElementFieldsTemplate(fieldDefList_t *fieldDefs, 
									  Bid *bidPtr, ipap_message *mes)
{
	
#ifdef DEBUG
    log->dlog(ch, "Starting addElementFieldsTemplate");
#endif
	
	int nfields = 0;
	uint16_t bidTemplateId;
	map<string,fieldDefItem_t> fields;
	
	// Loop through the elements in order to put them in a data record
	elementListIter_t elemListIter;
	for (elemListIter = bidPtr->getElements()->begin(); 
			elemListIter != bidPtr->getElements()->end(); ++elemListIter){
		
		fieldListIter_t fieldIter;
		
		for (fieldIter = (elemListIter->second).begin(); 
				fieldIter != (elemListIter->second).end(); ++fieldIter){
			fieldDefItem_t fItem = findField(fieldDefs, fieldIter->name);
			if ((fItem.name).empty()){
				ostringstream s;
				s << "Bid Message Parser: Field name:" << fieldIter->name
				  << "is not parametrized";
				throw Error(s.str()); 
			}
			else{
				fields[fieldIter->name] = fItem;
			}
		}
	}

	// Loop through the fields in order to include them in the template
	nfields = fields.size() + 2; // we include additionally IDBID and IDRECORD.
	bidTemplateId = mes->new_data_template( nfields, IPAP_SETID_BID_TEMPLATE );
	//put the BID name
	mes->add_field(bidTemplateId, 0, IPAP_FT_IDBID);
	// put the RECORD ID.
	mes->add_field(bidTemplateId, 0, IPAP_FT_IDRECORD);
	
	// Loop thought the fields to include and include them.
	map<string,fieldDefItem_t>::iterator fieldIter;
	for (fieldIter = fields.begin(); fieldIter != fields.end(); ++fieldIter){
		mes->add_field(bidTemplateId, fieldIter->second.eno, fieldIter->second.ftype);
	}	

#ifdef DEBUG
    log->dlog(ch, "Ending addElementFieldsTemplate");
#endif	

	return bidTemplateId;
}

//! Add required field for the bid's option template 									 
uint16_t MAPIBidParser::addFieldsOptionTemplate(fieldDefList_t *fieldDefs, 
												Bid *bidPtr, 
												ipap_message *mes)
{

#ifdef DEBUG
    log->dlog(ch, "Starting addFieldsOptionTemplate");
#endif

	uint16_t bidOptionTemplateId;
	
	// Add the option bid template
	int nfields = 6;
	bidOptionTemplateId = mes->new_data_template( nfields, IPAP_OPTNS_BID_TEMPLATE );
	//put the name
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_IDBID);
	// put the starttime
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_STARTSECONDS);
	// put the endtime
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_ENDSECONDS);
	// put the interval.
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_INTERVALSECONDS);
	// put the auction id.
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_IDAUCTION);
	// put the record id.
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_IDRECORD);

#ifdef DEBUG
    log->dlog(ch, "Ending addFieldsOptionTemplate");
#endif

	return bidOptionTemplateId;
}
												

void MAPIBidParser::addElementRecord(string bidId,
									 string elementId, 
									 fieldList_t *fieldList,
									 fieldDefList_t *fieldDefs,
									 uint16_t bidTemplateId, 
									 ipap_message *mes )
{

#ifdef DEBUG
    log->dlog(ch, "Starting addElementRecord");
#endif	
	
	ipap_data_record dataElement(bidTemplateId);
	ipap_field idBidF = mes->get_field_definition( 0, IPAP_FT_IDBID );
	ipap_value_field fvalue1 = idBidF.get_ipap_value_field( 
									strdup(bidId.c_str()), bidId.size() );
	dataElement.insert_field(0, IPAP_FT_IDBID, fvalue1);

	// Add the Record Id
	ostringstream ss;
	ss << elementId;
	string recordId = ss.str();
	ipap_field idRecordF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	ipap_value_field fvalue2 = idRecordF.get_ipap_value_field( 
									strdup(recordId.c_str()), recordId.size() );
	dataElement.insert_field(0, IPAP_FT_IDRECORD, fvalue2);
	
	// Add the rest of the field within the element.
	fieldListIter_t fieldListIter;
	for (fieldListIter = fieldList->begin(); fieldListIter != fieldList->end(); ++fieldListIter)
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
	
	mes->include_data(bidTemplateId, dataElement);

#ifdef DEBUG
    log->dlog(ch, "Ending addElementRecord");
#endif

}


void MAPIBidParser::addOptionRecord(string bidId,
									int recordId,
									bid_auction_t bAuct, 
									uint16_t bidTemplateId, 
									ipap_message *mes )
{

#ifdef DEBUG
    log->dlog(ch, "Starting addOptionRecord");
#endif
	
	ipap_data_record dataOption(bidTemplateId);
	ipap_field idBidF = mes->get_field_definition( 0, IPAP_FT_IDBID );
	ipap_value_field fvalue1 = idBidF.get_ipap_value_field( 
									strdup(bidId.c_str()), bidId.size() );
	dataOption.insert_field(0, IPAP_FT_IDBID, fvalue1);

	// Add the Record Id
	ostringstream ss;
	ss << recordId;
	string srecordId = ss.str();
	ipap_field idRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	ipap_value_field fvalue2 = idRecordIdF.get_ipap_value_field( 
									strdup(srecordId.c_str()), srecordId.size() );
	dataOption.insert_field(0, IPAP_FT_IDRECORD, fvalue2);
	
	// Add the auction Id
	string idAuctionS = bAuct.auctionSet + "." + bAuct.auctionName;
	ipap_field idAuctionF = mes->get_field_definition( 0, IPAP_FT_IDAUCTION );
	ipap_value_field fvalue3 = idAuctionF.get_ipap_value_field( 
								strdup(idAuctionS.c_str()), idAuctionS.size() );
	dataOption.insert_field(0, IPAP_FT_IDAUCTION, fvalue3);
	
	// Add the start datetime
	assert (sizeof(uint64_t) >= sizeof(time_t));
	time_t time = bAuct.start;	
	uint64_t timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_field idStartF = mes->get_field_definition( 0, IPAP_FT_STARTSECONDS );
	ipap_value_field fvalue4 = idStartF.get_ipap_value_field( timeUint64 );
	dataOption.insert_field(0, IPAP_FT_STARTSECONDS, fvalue4);
	
	// Add the endtime
	ipap_field idStopF = mes->get_field_definition( 0, IPAP_FT_ENDSECONDS );
	time = bAuct.stop;	
	timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_value_field fvalue5 = idStopF.get_ipap_value_field( timeUint64 );
	dataOption.insert_field(0, IPAP_FT_ENDSECONDS, fvalue5);
	
	// Add the interval.
	assert (sizeof(uint64_t) >= sizeof(unsigned long));
	uint64_t uinter = static_cast<uint64_t>(bAuct.interval);
	ipap_field idIntervalF = mes->get_field_definition( 0, IPAP_FT_INTERVALSECONDS );
	ipap_value_field fvalue6 = idIntervalF.get_ipap_value_field( uinter );
	dataOption.insert_field(0, IPAP_FT_INTERVALSECONDS, fvalue6);
		
	mes->include_data(bidTemplateId, dataOption);

#ifdef DEBUG
    log->dlog(ch, "Ending addOptionRecord");
#endif

}





auction::fieldList_t MAPIBidParser::readBidData( ipap_template *templ, 
									  fieldDefList_t *fieldDefs,
									  fieldValList_t *fieldVals,
									  ipap_data_record &record,
									  string &bidSet,
									  string &bidName,
									  string &elementName )
{

#ifdef DEBUG
    log->dlog(ch, "Starting readBidData");
#endif

	string recordName, bidId;
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
		}
		else{
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
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDRECORD)){
				elementName = field.writeValue(dFieldValue);
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
    log->dlog(ch, "Ending readBidData");
#endif
	
	return fields;
}

bid_auction_t MAPIBidParser::readBidOptionData(ipap_template *templ, 
											   fieldDefList_t *fieldDefs,
											   ipap_data_record &record)
{

#ifdef DEBUG
    log->dlog(ch, "Starting readBidOptionData");
#endif

	bid_auction_t auction;
	string auctionName, bidName;
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

			// Search the auction name field.
			if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDBID)){
				continue;
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDRECORD)){
				continue;
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_IDAUCTION)){
				auctionName = field.writeValue(dFieldValue);
				parseName(auctionName, auction.auctionSet, auction.auctionName);
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_STARTSECONDS)){
				
				auction.start = (time_t) dFieldValue.get_value_int64();
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_ENDSECONDS)){
				auction.stop = (time_t) dFieldValue.get_value_int64();
			}
			else if ((kField.get_eno() == 0) && 
				  (kField.get_ftype()== IPAP_FT_INTERVALSECONDS)){
				auction.interval = (int) dFieldValue.get_value_int32();
			}
			else {
				ostringstream s;
				s << "Bid Message Parser: Field eno:" << kField.get_eno();
				s << "fType:" << kField.get_ftype();
				s << " was not expected to be included ";
				throw Error(s.str()); 
			}
		}
	}

#ifdef DEBUG
    log->dlog(ch, "Ending readBidOptionData");
#endif
	
	return auction;
}


void MAPIBidParser::parse(fieldDefList_t *fields, 
						  fieldValList_t *fieldVals,
						  ipap_message *message,
						  bidDB_t *bids,
						  BidIdSource *idSource,
						  ipap_message *messageOut )
{

#ifdef DEBUG
    log->dlog(ch, "Starting parse");
#endif

    string bidSet, bidName;
    elementList_t elements;
    bidAuctionList_t auctions;

    try {
		// Read the template bid
		ipap_template *templBid = readTemplate(message, IPAP_SETID_BID_TEMPLATE);
		
		// Read the option bid template
		ipap_template *templOptBid = readTemplate(message, IPAP_OPTNS_BID_TEMPLATE); 
			
		// Read the record data associated with the data bid template.
		if (templBid != NULL){
			dataRecordList_t dRecordList = readDataRecords(message, templBid->get_template_id());
			
			if (dRecordList.size() == 0){
				throw Error("Bid Message Parser: a data bid template was not given"); 
			} else {
				dateRecordListIter_t dataIter;
				for (dataIter = dRecordList.begin(); dataIter != dRecordList.end(); ++dataIter)
				{
					string elementName;
					fieldList_t elemtFields = readBidData(templBid, fields, fieldVals, *dataIter, bidSet, bidName, elementName );
					elements[elementName] = elemtFields;
				}
			}
		} else {
			throw Error("Bid Message Parser: missing data bid template data"); 
		}
		
		
		// Read the option data record associated with the option data bid template.
		if (templOptBid != NULL){
			
			dataRecordList_t dOptRecordList = readDataRecords(message, templOptBid->get_template_id());
			if (dOptRecordList.size() == 0){
				throw Error("Bid Message Parser: an option data template was not given"); 
			} else {
				dateRecordListIter_t dataIter;
				for (dataIter = dOptRecordList.begin(); dataIter != dOptRecordList.end(); ++dataIter)
				{
					bid_auction_t bidAuction = readBidOptionData(templOptBid, fields, *dataIter);
					auctions[bidAuction.getId()] = bidAuction;
				}
			}		
		} else{
			throw Error("Bid Message Parser: missing option data template data"); 
		}
    
     
        Bid *b = new Bid(bidSet, bidName, elements, auctions);
        bids->push_back(b);

#ifdef DEBUG
		// debug info
		log->dlog(ch, "bid %s.%s - %s", bidName.c_str(), bidName.c_str(), (b->getInfo()).c_str());
#endif
            
    } catch (Error &e) {
       log->elog(ch, e);
       throw e;
    }
}


/* ------------------------- getMessage ------------------------- */
ipap_message * MAPIBidParser::get_ipap_message(Bid *bidPtr, 
											   fieldDefList_t *fieldDefs)
{
#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message");
#endif	

	// We assume that both templates were already sent to the other part.
	
	uint16_t bidOptionTemplateId, bidTemplateId;
	ipap_message *mes = new ipap_message();
	
	bidOptionTemplateId = addFieldsOptionTemplate(fieldDefs, bidPtr, mes);
	
#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message 1");
#endif
	
	// Loop through the fields in order to put them in a bid Template record
	bidTemplateId = addElementFieldsTemplate(fieldDefs, bidPtr, mes);
	
	// Build the Id of the bid.
	string bidId = bidPtr->getSetName() + "." + bidPtr->getBidName();
		
	// Loop through the elements in order to put them in a data record
	elementListIter_t elemIter;
	for (elemIter = bidPtr->getElements()->begin(); 
			elemIter != bidPtr->getElements()->end(); ++elemIter)
	{
		addElementRecord(bidId, elemIter->first, &(elemIter->second), fieldDefs, bidTemplateId, mes );
	}

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message 2");
#endif
	
	// Loop through the related auctions to include them in a option record.
	bidAuctionListIter_t auctionListIter;
	int recordId = 1;
	for (auctionListIter = bidPtr->getAuctions()->begin(); 
			auctionListIter != bidPtr->getAuctions()->end(); ++auctionListIter)
	{
		addOptionRecord(bidId, recordId, auctionListIter->second, bidOptionTemplateId, mes);
		recordId = recordId + 1;
	}
	
#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_message");
#endif	
	
	return mes;
}											

vector<ipap_message *> 
MAPIBidParser::get_ipap_messages(fieldDefList_t *fieldDefs, 
									  bidDB_t *bids)
{

#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_messages");
#endif	
	
	vector<ipap_message *> vct_return;
	bidDBIter_t bidIter;
	for (bidIter=bids->begin(); bidIter!=bids->end(); ++bidIter){
		Bid *b = *bidIter;
		ipap_message *mes =get_ipap_message(b, fieldDefs);
		vct_return.push_back(mes);
	}

#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_messages");
#endif
	
	return vct_return;
		
}
