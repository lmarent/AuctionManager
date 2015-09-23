
/*! \file   Field.cpp

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
    Fields in the system.

    $Id: Field.cpp 748 2015-08-24 8:47:00Z amarentes $
*/

#include "Field.h"

using namespace auction;

field_t::field_t(const field_t &param)
{
	name = param.name;
	type = param.type;
	mtype = param.mtype;
	len = param.len;
	//! number of values
	cnt = param.cnt;

	vector<FieldValue>::const_iterator fval_iter;
	for ( fval_iter = param.value.begin(); fval_iter != param.value.end(); ++fval_iter )
	{
		value.push_back(*fval_iter);
	}
}

void field_t::parseFieldValue(string val)
{

    int n;
	
	// Initialize the values for the field.
	for (int i=0 ; i < MAX_FIELD_SET_SIZE; i++)
	{
		FieldValue fielvalue;
		value.push_back(fielvalue);
	}
	
			
    if (val == "*") {
        mtype = FT_WILD;
        cnt = 1;
    } else if ((n = val.find("-")) > 0) {
        mtype = FT_RANGE;
        value[0] = FieldValue(type, val);
        value[1] = FieldValue(type, val);
        cnt = 2;
    } else if ((n = val.find(",")) > 0) {
        int lastn = 0;
        int c = 0;

        n = -1;
        mtype = FT_SET;
        while (((n = val.find(",", lastn)) > 0) && (c<(MAX_FIELD_SET_SIZE-1))) {
            value[c] = FieldValue(type, val);
            c++;
            lastn = n+1;
        }
        value[c] = FieldValue(type, val);
        cnt = c+1;
        if ((n > 0) && (cnt == MAX_FIELD_SET_SIZE)) {
            throw Error("more than %d field specified in set", MAX_FIELD_SET_SIZE);
        }
    } else {
        mtype = FT_EXACT;
        value[0] = FieldValue(type, val);
        cnt = 1;
    }
    
}


string field_t::getInfo(void)
{
	std::stringstream output;
	
	output << "name:" << name 
		   << " type:";
		    
    switch (mtype) {
    case FT_EXACT:
        output << "Exact";
        break;
    case FT_RANGE:
        output << "Range";
        break;
    case FT_SET:
        output << "Set";
        break;
    case FT_WILD:
        output << "Wild";
        break;
    default:
        output << "unknown";
    }
	
	output << " len:" << len
		   << " cnt:" << cnt << endl;
	
	vector<FieldValue>::iterator val_iter;
	for (val_iter = value.begin(); val_iter != value.end(); ++val_iter){
		if (!val_iter->getType().empty())
			output << (*val_iter).getInfo();
	}
	
	return output.str();
}
