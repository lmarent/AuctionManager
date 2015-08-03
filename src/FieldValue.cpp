
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

#include "FieldValue.h"
#include "Error.h"
#include "ParserFcts.h"


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

FieldValue::FieldValue(const FieldValue &param)
{
    ftype = param.ftype;
	len = param.len;
	val = param.val;
}

FieldValue::~FieldValue()
{

}


FieldValue& 
FieldValue::operator= (const FieldValue &param)
{
    ftype = param.ftype;
	len = param.len;
	val = param.val;
	return *this;
}

bool
FieldValue::operator==(const FieldValue &param)
{
	if (ftype.compare(param.ftype) != 0)
		return false;
	
	if (len != param.len)
		return false;
	
	if (val.compare(param.val))
		return false;
	
	return true;
}

bool
FieldValue::operator!=(const FieldValue &param)
{
	return !(operator==(param));
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
		return 8;
	} else if (type == "Double") {
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
