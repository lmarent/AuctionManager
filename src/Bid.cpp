
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

#include "Bid.h"
#include "Error.h"


/* ------------------------- Bid ------------------------- */

Bid::Bid(int _uid, string sname, string rname, elementList_t &elements)
  : uid(_uid), bidName(rname), setName(sname), state(BS_NEW), elementList(elements)
{
    unsigned long duration;

    log = Logger::getInstance();
    ch = log->createChannel("Bid");

#ifdef DEBUG
    log->dlog(ch, "Bid constructor");
#endif    

}

Bid::Bid(int id, const Bid &rhs)
  :uid(id), state(BS_NEW)
{
	bidName = rhs.bidName;
	setName = rhs.setName;
	startTime = rhs.startTime;
	endTime = rhs.endTime;
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
	
}

Bid::~Bid()
{
#ifdef DEBUG
    log->dlog(ch, "Bid destructor");
#endif    

}

/* ------------------------- getElements ------------------------- */

elementList_t *Bid::getElements()
{
    return &elementList;
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
	output << "uid:" << getUId() << std::endl;
	output << "StartTime:" << getStartTime() 
			<< " EndTime:" << getEndTime() 
			<< " NbrElements:" << elementList.size()
			<< std::endl;
	
	elementListIter_t iter;
	for (iter = elementList.begin(); iter != elementList.end(); ++iter ){
		output << toString(*iter) << std::endl;
	}
	return output.str();
}
