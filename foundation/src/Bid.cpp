
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

Bid::Bid( string sname, string rname, elementList_t &elements,
		 bidAuctionList_t &ba)
  : uid(0), setName(sname), bidName(rname), state(BS_NEW), 
	elementList(elements), auctionList(ba)
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
	setName = rhs.setName;
	bidName = rhs.bidName;
	
	// Copy elements part of the bid.
	elementListConstIter_t iter;
	for (iter = (rhs.elementList).begin(); iter != (rhs.elementList).end(); ++iter ){
		element_t elem;
		elem.name = (*iter).name;
		fieldListconstIter_t fielditer;
		for ( fielditer = ((*iter).fields).begin(); 
				fielditer != ((*iter).fields).end(); ++fielditer ){
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
			elem.fields.push_back(field);
		}
		elementList.push_back(elem);
	}
	
	// Copy auctions part of the bid
	bidAuctionListConstIter_t iter_act;
	for (iter_act = (rhs.auctionList).begin(); iter_act != (rhs.auctionList).end(); ++iter_act )
	{
		bid_auction_t auction;
		auction.auctionSet = iter_act->auctionSet;
		auction.auctionName = iter_act->auctionName;
		auction.start = iter_act->start;
		auction.stop = iter_act->stop;

		bidIntervalListConstIter_t inter_iter;
		for ( inter_iter = iter_act->intervals.begin(); 
					inter_iter != iter_act->intervals.end(); ++inter_iter){

			interval_t ientry;
           	ientry.interval = inter_iter->interval;
           	ientry.align = inter_iter->align;
			auction.intervals.push_back(ientry);
		}
				
		
		// Copy Misc items.
		miscListConstIter_t misc_iter;
		for ( misc_iter = iter_act->miscList.begin(); 
					misc_iter != iter_act->miscList.end(); ++misc_iter){
			configItem_t item;
			item.group = (misc_iter->second).group;
			item.module = (misc_iter->second).module;
			item.name = (misc_iter->second).name;
			item.value = (misc_iter->second).value;
			item.type = (misc_iter->second).type;
			
			auction.miscList[misc_iter->first] = item;
		}
		
		
		auctionList.push_back(auction);
	}
	
}

Bid::~Bid()
{
#ifdef DEBUG
    log->dlog(ch, "Bid destructor");
#endif    

}

string Bid::toString(element_t &elem)
{
	std::stringstream output;
	
	fieldListIter_t iter;
	
	output << "Element Name:" << elem.name << std::endl;
	output << " Field number:" << elem.fields.size() << std::endl;
	for (iter = elem.fields.begin(); iter != elem.fields.end(); ++iter)
	{
		output << "field name:" << (*iter).name 
			   << " type:" << (*iter).type
			   << " len:" << (*iter).len
			   << " Value:" << ((*iter).value[0]).getValue()
			   << std::endl;
	}
	
	return output.str();

}	

string Bid::getInfo()
{
	std::stringstream output;

	output << "Set:" << getSetName() 
		   << " Name:" << getBidName();

	output 	<< " NbrElements:" << elementList.size()
			<< std::endl;
	
	elementListIter_t iter;
	for (iter = elementList.begin(); iter != elementList.end(); ++iter ){
		output << toString(*iter) << std::endl;
	}
	
	output  << "Nbr Auctions:" << auctionList.size();
	bidAuctionListIter_t ait;
	for (ait = auctionList.begin(); ait != auctionList.end(); ++ait ){
		output << " AuctionSet:" << ait->auctionSet
			   << " AuctionName:" << ait->auctionName 
			   << " Start: " << Timeval::toString(ait->start) 
			   << " Stop:" << Timeval::toString(ait->stop) << std::endl;

		bidIntervalListIter_t inter_iter;
		for ( inter_iter = ait->intervals.begin(); 
				inter_iter != ait->intervals.end(); ++inter_iter){
			output << "Interval:" << inter_iter->interval 
				   << "Align:" << inter_iter->align 
				   << endl;
		}
			   		
		miscListIter_t misc_iter;
		for ( misc_iter = ait->miscList.begin(); misc_iter != ait->miscList.end(); ++misc_iter){
			output << "miscItem:" << misc_iter->first 
				   << "Value:" << (misc_iter->second).value
				   << endl;
		}
		 
	}
	
	
	return output.str();
}


