
/*!  \file   MAPIIpApMessageParser.cpp

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
    common methods for parsers from a API ipap_message

    $Id: MAPIIpApMessageParser.cpp 748 2015-07-23 17:30:00Z amarentes $
*/

#include "ParserFcts.h"
#include "IpApMessageParser.h"

using namespace auction;

/* ------------------------- lookup ------------------------- */
string 
IpApMessageParser::lookup(fieldValList_t *fieldVals, string fvalue, field_t *f)
{
    fieldValListIter_t iter2 = fieldVals->find(fvalue);
    if ((iter2 != fieldVals->end()) && (iter2->second.type == f->type)) {
        // substitute field value
        fvalue = iter2->second.svalue;
    }

    return fvalue;
}

void 
IpApMessageParser::parseFieldValue(fieldValList_t *fieldVals, string value, field_t *f)
{
    int n;
	
	// Initialize the values for the field.
	for (int i=0 ; i < MAX_FIELD_SET_SIZE; i++)
	{
		FieldValue fielvalue;
		f->value.push_back(fielvalue);
	}    

    if (value == "*") {
        f->mtype = FT_WILD;
        f->cnt = 1;
    } else if ((n = value.find("-")) > 0) {
        f->mtype = FT_RANGE;
        f->value[0] = FieldValue(f->type, lookup(fieldVals, value.substr(0,n),f));
        f->value[1] = FieldValue(f->type, lookup(fieldVals, value.substr(n+1, value.length()-n+1),f));
        f->cnt = 2;
    } else if ((n = value.find(",")) > 0) {
        int lastn = 0;
        int c = 0;

        n = -1;
        f->mtype = FT_SET;
        while (((n = value.find(",", lastn)) > 0) && (c<(MAX_FIELD_SET_SIZE-1))) {
            f->value[c] = FieldValue(f->type, lookup(fieldVals, value.substr(lastn, n-lastn),f));
            c++;
            lastn = n+1;
        }
        f->value[c] = FieldValue(f->type, lookup(fieldVals, value.substr(lastn, n-lastn),f));
        f->cnt = c+1;
        if ((n > 0) && (f->cnt == MAX_FIELD_SET_SIZE)) {
            throw Error("more than %d field specified in set", MAX_FIELD_SET_SIZE);
        }
    } else {
        f->mtype = FT_EXACT;
        f->value[0] = FieldValue(f->type, lookup(fieldVals, value,f));
        f->cnt = 1;
    }
}


/* ------------------------- parseName ------------------------- */
void 
IpApMessageParser::parseName(string id, string &set, string &name)
{

    if (id.empty()) {
        throw Error("malformed identifier %s, "
                    "use <identifier> or <set>.<identifier> ",
                    id.c_str());
    }

	size_t found = id.find(".");
	if (found!=std::string::npos){
		set = id.substr (0,found);
		name = id.substr(found+1, id.size()-found);
	}
	else
	{
		name = id;
	}
}

/* ------------------------- getMiscVal ------------------------- */
string 
IpApMessageParser::getMiscVal(miscList_t *_miscList, string name)
{
    miscListIter_t iter;

    iter = _miscList->find(name);
    if (iter != _miscList->end()) {
        string value = iter->second.value;
        // use lower case internally
		transform(value.begin(), value.end(), value.begin(), ToLower());
        return value;
    } else {
        return "";
    }
}


/* ------------------------- findField ------------------------- */
fieldDefItem_t 
IpApMessageParser::findField(fieldDefList_t *fieldDefs, int eno, int ftype)
{
	fieldDefItem_t val_return;
		
	fieldDefListIter_t iter;
	for (iter=fieldDefs->begin(); iter!=fieldDefs->end(); ++iter)
	{
		if (((iter->second).eno == eno) && ((iter->second).ftype == ftype)){
			val_return = iter->second;
			break;
		}
	}
	return val_return;
}


/* ------------------------- findField ------------------------- */
fieldDefItem_t 
IpApMessageParser::findField(fieldDefList_t *fieldDefs, string fname)
{
	fieldDefItem_t val_return;
	fieldDefListIter_t iter = fieldDefs->find(fname);
	if (iter != fieldDefs->end()){
		val_return = iter->second;
	}
	
	return val_return;
}


/* ------------------------- readTemplate ------------------------- */
ipap_template * 
IpApMessageParser::readTemplate(ipap_message * message,  
									ipap_templ_type_t type)
{
	std::list<int> tempList = message->get_template_list();
	std::list<int>::iterator tempIter;
	
	for (tempIter = tempList.begin(); tempIter != tempList.end(); ++tempIter)
	{
		ipap_template *templ = message->get_template_object(*tempIter);
		if (templ->get_type() == type){
			return templ;
		}
	}
	
	return NULL;
	
}

/* ------------------------- readDataRecords ------------------------- */
dataRecordList_t 
IpApMessageParser::readDataRecords(ipap_message * message, 
									   uint16_t templId)
{
	
	dataRecordList_t dListReturn;
	dateRecordListConstIter_t dataIter;
	for(dataIter = message->begin(); dataIter != message->end(); ++dataIter)
	{
		ipap_data_record dRecord = *dataIter;
		
		if (dRecord.get_template_id() == templId){
			dListReturn.push_back(dRecord);
		}
	}

	return dListReturn;
}
