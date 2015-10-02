
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

bool 
bid_auction_t::operator==(const bid_auction_t &rhs)
{
	if (auctionSet.compare(rhs.auctionSet) != 0)
		return false;
		
	if (auctionName.compare(rhs.auctionName) != 0)
		return false;

	if (interval != rhs.interval)
		return false;

	if (align != rhs.align)
		return false;

	if (start != rhs.start)
		return false;

	if (stop != rhs.stop)
		return false;

	return true;
}

bool 
bid_auction_t::operator!=(const bid_auction_t &param)
{
	return !(operator==(param));
}


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
		fieldList_t fieldList;
		fieldListconstIter_t fielditer;
		
		for ( fielditer = (iter->second).begin(); 
				fielditer != (iter->second).end(); ++fielditer ){
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
			fieldList.push_back(field);
		}
		elementList[iter->first] = fieldList;
	}
	
	// Copy auctions part of the bid
	bidAuctionListConstIter_t iter_act;
	for (iter_act = (rhs.auctionList).begin(); iter_act != (rhs.auctionList).end(); ++iter_act )
	{
		bid_auction_t auction;
		auction.auctionSet = (iter_act->second).auctionSet;
		auction.auctionName = (iter_act->second).auctionName;
		auction.start = (iter_act->second).start;
		auction.stop = (iter_act->second).stop;
		auction.interval = (iter_act->second).interval;
		auction.align = (iter_act->second).align;
		string actId = auction.getId();
		auctionList[actId] = auction;
	}
	
}

Bid::~Bid()
{
#ifdef DEBUG
    log->dlog(ch, "Bid destructor");
#endif    

}

void Bid::deleteAuction(string aset, string aName)
{
	bidAuctionListIter_t auctListIter;
	for (auctListIter = auctionList.begin(); 
			auctListIter != auctionList.end(); ++auctListIter){
		if ((strcmp(aset.c_str(), ((auctListIter->second).auctionSet).c_str()) == 0) &&
			(strcmp(aName.c_str(), ((auctListIter->second).auctionName).c_str()) == 0)){
			auctionList.erase(auctListIter);
			break;
		}
	}
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
	
	output  << "Nbr Auctions:" << auctionList.size();
	bidAuctionListIter_t ait;
	for (ait = auctionList.begin(); ait != auctionList.end(); ++ait ){
		output << " AuctionSet:" << (ait->second).auctionSet
			   << " AuctionName:" << (ait->second).auctionName 
			   << " Start: " << Timeval::toString((ait->second).start) 
			   << " Stop:" << Timeval::toString((ait->second).stop) 
			   << " Interval: " <<  ((ait->second).interval)
			   << " Align: " <<  ((ait->second).align)
			   << std::endl;		 
	}
	
	
	return output.str();
}

bool Bid::operator==(const Bid &rhs)
{

#ifdef DEBUG
    log->dlog(ch, "Starting operator == ");
#endif  

	if (setName.compare(rhs.setName) != 0 )
		return false;
		
	if (bidName.compare(rhs.bidName) != 0 )
		return false;


#ifdef DEBUG
    log->dlog(ch, "operator == equal bid general info");
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

	if (auctionList.size() != rhs.auctionList.size())
		return false;
		
	bidAuctionListIter_t bidActionIter;
	for (bidActionIter = auctionList.begin(); bidActionIter != auctionList.end(); ++bidActionIter ){
		string actId = bidActionIter->first;
		bidAuctionListConstIter_t rhsActIter = rhs.auctionList.find(actId);
		if (rhsActIter == rhs.auctionList.end()){
			return false;
		} else {
			if (bidActionIter->second != rhsActIter->second){
				return false;
			}
		}
	}
	
#ifdef DEBUG
    log->dlog(ch, "operator == equal auction info");
#endif
	
	return true;
}

bool 
Bid::operator!=(const Bid &param)
{
	return !(operator==(param));
}
