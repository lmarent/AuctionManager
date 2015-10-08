
/*!  \file   MAPIResourceRequestParser.cpp

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

    $Id: MAPIResourceRequestParser.cpp 2015-07-24 15:14:00 amarentes $
*/

#include "ParserFcts.h"
#include "Constants.h"
#include "MAPIResourceRequestParser.h"
#include "Timeval.h"

using namespace auction;

MAPIResourceRequestParser::MAPIResourceRequestParser()
    : MAPIIpApMessageParser()
{
    log = Logger::getInstance();
    ch = log->createChannel("MAPIResourceRequestParser" );
    
#ifdef DEBUG
    log->dlog(ch, "Constructor MAPIResourceRequestParser");
#endif    
}


//! Add required field for the bid's option template 									 
uint16_t MAPIResourceRequestParser::addFieldsOptionTemplate(fieldDefList_t *fieldDefs, 
												ipap_message *mes)
{

#ifdef DEBUG
    log->dlog(ch, "Starting addFieldsOptionTemplate");
#endif

	uint16_t bidOptionTemplateId;
	
	// Add the option bid template
	int nfields = 5;
	bidOptionTemplateId = mes->new_data_template( nfields, IPAP_OPTNS_AUCTION_TEMPLATE );

	// put the AUCTIONID.
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_IDAUCTION);
	// put the RECORDID.
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_IDRECORD);
	// put the ResourceID
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_IDRESOURCE);
	// put the starttime
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_STARTSECONDS);
	// put the endtime
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_ENDSECONDS);
	// put the interval
	mes->add_field(bidOptionTemplateId, 0, IPAP_FT_INTERVALSECONDS);


#ifdef DEBUG
    log->dlog(ch, "Ending addFieldsOptionTemplate");
#endif

	return bidOptionTemplateId;
}
												

void MAPIResourceRequestParser::addOptionRecord(string recordId,
												string resourceId,
												resourceReq_interval_t interval, 
												uint16_t bidTemplateId, 
												ipap_message *mes )
{

#ifdef DEBUG
    log->dlog(ch, "Starting addOptionRecord");
#endif
	
	ipap_data_record dataOption(bidTemplateId);

	// Add the Auction Id
	ipap_field idAuctionIdF = mes->get_field_definition( 0, IPAP_FT_IDAUCTION );
	ipap_value_field fvalue0 = idAuctionIdF.get_ipap_value_field( 
						strdup(NO_AUCTION_VALUE.c_str()), NO_AUCTION_VALUE.size() );
	dataOption.insert_field(0, IPAP_FT_IDAUCTION, fvalue0);

	// Add the Record Id
	ipap_field idRecordIdF = mes->get_field_definition( 0, IPAP_FT_IDRECORD );
	ipap_value_field fvalue1 = idRecordIdF.get_ipap_value_field( 
									strdup(recordId.c_str()), recordId.size() );
	dataOption.insert_field(0, IPAP_FT_IDRECORD, fvalue1);

	// Add the Resource Id
	ipap_field resourceIdF = mes->get_field_definition( 0, IPAP_FT_IDRESOURCE );
	ipap_value_field fvalue2 = resourceIdF.get_ipap_value_field( 
									strdup(resourceId.c_str()), resourceId.size() );
	dataOption.insert_field(0, IPAP_FT_IDRESOURCE, fvalue2);
	
	// Add the start datetime
	assert (sizeof(uint64_t) >= sizeof(time_t));
	time_t time = interval.start;	
	uint64_t timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_field idStartF = mes->get_field_definition( 0, IPAP_FT_STARTSECONDS );
	ipap_value_field fvalue3 = idStartF.get_ipap_value_field( timeUint64 );
	dataOption.insert_field(0, IPAP_FT_STARTSECONDS, fvalue3);
	
	// Add the endtime
	ipap_field idStopF = mes->get_field_definition( 0, IPAP_FT_ENDSECONDS );
	time = interval.stop;	
	timeUint64 = *reinterpret_cast<uint64_t*>(&time);
	ipap_value_field fvalue4 = idStopF.get_ipap_value_field( timeUint64 );
	dataOption.insert_field(0, IPAP_FT_ENDSECONDS, fvalue4);
	
	// Add the interval.
	assert (sizeof(uint64_t) >= sizeof(unsigned long));
	uint64_t uinter = static_cast<uint64_t>(interval.interval);
	ipap_field idIntervalF = mes->get_field_definition( 0, IPAP_FT_INTERVALSECONDS );
	ipap_value_field fvalue5 = idIntervalF.get_ipap_value_field( uinter );
	dataOption.insert_field(0, IPAP_FT_INTERVALSECONDS, fvalue5);
		
	mes->include_data(bidTemplateId, dataOption);

#ifdef DEBUG
    log->dlog(ch, "Ending addOptionRecord");
#endif

}



/* ------------------------- getMessage ------------------------- */
ipap_message * 
MAPIResourceRequestParser::get_ipap_message(fieldDefList_t *fieldDefs, 
											string recordId,
											string resourceId,
											resourceReq_interval_t interval )
{
#ifdef DEBUG
    log->dlog(ch, "Starting get_ipap_message");
#endif	

	// We assume that both templates were already sent to the other part.
	
	uint16_t bidOptionTemplateId;
	ipap_message *mes = new ipap_message();
	
	bidOptionTemplateId = addFieldsOptionTemplate(fieldDefs, mes);
				
	// Loop through the related auctions to include them in a option record.
	addOptionRecord(recordId, resourceId, interval, bidOptionTemplateId, mes);
	
#ifdef DEBUG
    log->dlog(ch, "Ending get_ipap_message");
#endif	
	
	return mes;
}											

