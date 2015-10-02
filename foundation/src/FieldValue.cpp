
/*!  \file   FieldValue.cpp

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
    Class used to store field values with
    flexible length

    $Id: FieldValue.cpp 748 2015-07-23 15:30:00Z amarentes $
*/

#include "ParserFcts.h"
#include "FieldValue.h"
#include "Error.h"

using namespace auction;

FieldValue::FieldValue(string type, string _value)
    : ftype(type), len(0)
{
   
    if (type == "UInt8") {
        (unsigned char) ParserFcts::parseULong(_value);
        val = _value;
    } else if (type == "SInt8") {
        (char) ParserFcts::parseULong(_value);
        val = _value;
    } else if (type == "UInt16") {
        (unsigned short) ParserFcts::parseULong(_value);
        val = _value;
    } else if (type == "SInt16") {
        (short) ParserFcts::parseLong(_value);
        val = _value;
    } else if (type == "UInt32") {
        (unsigned long) ParserFcts::parseULong(_value);
        val = _value;
    } else if (type == "SInt32") {
        (long) ParserFcts::parseULong(_value);
        val = _value;
    } else if (type == "UInt64") {
        (unsigned long long) ParserFcts::parseULLong(_value);
        val = _value;
    } else if (type == "Binary") {
        (int) ParserFcts::parseBool(_value.c_str());
        val = _value;
    } else if (type == "String") {
        val = _value;
    } else if (type == "Float") {
		(float) ParserFcts::parseFloat(_value.c_str());
		val = _value;
	} else if (type == "Double") {
		(double) ParserFcts::parseDouble(_value.c_str());
		val = _value;
	}
    else {
        throw Error("Unsupported type for field value: %s", type.c_str());
    }
}


bool
FieldValue::equal(const FieldValue &param, string type)
{
	if (ftype.compare(param.ftype) != 0){
		return false;
	}
	
	if (len != param.len){
		return false;
	}

	// Compare the value. It should compare by type.

	if (type == "UInt8") {
		unsigned char ls1 = (unsigned char) ParserFcts::parseULong(val);
		unsigned char rs1 = (unsigned char) ParserFcts::parseULong(param.val);
		if (ls1 !=rs1) return false;
	} 
	else if (type == "SInt8") {
		char ls2 = (char) ParserFcts::parseULong(val);
		char rs2 = (char) ParserFcts::parseULong(param.val);
		if (ls2 !=rs2) return false;
	} 
	else if (type == "UInt16") {
		unsigned short ls3 = (unsigned short) ParserFcts::parseULong(val);
		unsigned short rs3 = (unsigned short) ParserFcts::parseULong(param.val);
		if (ls3 !=rs3) return false;
	} 
	else if (type == "SInt16") {
		short ls4 = (short) ParserFcts::parseLong(val);
		short rs4 = (short) ParserFcts::parseLong(param.val);
		if (ls4 !=rs4) return false;
	} 
	else if (type == "UInt32") {
		unsigned long ls5 = (unsigned long) ParserFcts::parseULong(val);
		unsigned long rs5 = (unsigned long) ParserFcts::parseULong(param.val);
		if (ls5 !=rs5) return false;
	} 
	else if (type == "SInt32") {
		long ls6 = (long) ParserFcts::parseULong(val);
		long rs6 = (long) ParserFcts::parseULong(param.val);
		if (ls6 !=rs6) return false;
	} 
	else if (type == "UInt64") {
		unsigned long long ls7 = (unsigned long long) ParserFcts::parseULLong(val);
		unsigned long long rs7 = (unsigned long long) ParserFcts::parseULLong(param.val);
		if (ls7 !=rs7) return false;
	} 
	else if (type == "Binary") {
		int ls8 = (int) ParserFcts::parseBool(val.c_str());
		int rs8 = (int) ParserFcts::parseBool(param.val.c_str());
		if (ls8 !=rs8) return false;
	} 
	else if (type == "String") {
		if (val.compare(param.val) != 0) return false;
	} 
	else if (type == "Float") {
		float ls9 = (float) ParserFcts::parseFloat(val.c_str());
		float rs9 = (float) ParserFcts::parseFloat(param.val.c_str());
		if (ls9 !=rs9) return false;
	} 
	else if (type == "Double") {
		double ls10 = (double) ParserFcts::parseDouble(val.c_str());
		double rs10 = (double) ParserFcts::parseDouble(param.val.c_str());
		
		if (ls10 !=rs10){
			return false;
		}
	}
	else {
		throw Error("Unsupported type for field value: %s", type.c_str());
	}
	
	return true;
}

bool
FieldValue::notEqual(const FieldValue &param, string type)
{
	bool val_return = !(equal(param,type));
	return val_return;
}
   
int FieldValue::getTypeLength(string type)
{
    if ((type == "UInt8") || (type == "SInt8")) {
        return 1;
    } else if ((type == "UInt16") || (type == "SInt16")) {
        return 2;
    } else if ((type == "UInt32") || (type == "SInt32")) {
        return 4;
    } else if (type == "Float") {
		return 4;
	} else if ((type == "Double") || (type == "UInt64")) {
		return 8;		
	} else if (type == "Binary") {
        return 0;
    } else if (type == "String") {
        return 0;
    } else {
        throw Error("Unsupported type for field value: %s", type.c_str());
    }
}

string FieldValue::getString()
{        
    return val;
}

string FieldValue::getInfo()
{
	std::stringstream output;
	
	output << "type: " << ftype << " len:" << len 
		   << " val:" << val << endl; 
	
	return output.str();
}
