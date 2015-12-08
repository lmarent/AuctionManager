
/*!  \file   Field.h

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
	Definitions used to define a field
	code based on NETMATE

    $Id: Field.h 748 2015-08-21 9:09:00Z amarentes $

*/

#ifndef _FIELD_H_
#define _FIELD_H_


#include "stdincpp.h"
#include "Error.h"
#include "FieldValue.h"

namespace auction
{

//! field definition
typedef struct
{
    string name;
    string type;
    unsigned short len;
    
    // Data to associate the field with the IPAP MESSAGE FIELDS.
    int eno;
    int ftype;
} fieldDefItem_t;


//! field list
typedef map<string,fieldDefItem_t>            fieldDefList_t;
typedef map<string,fieldDefItem_t>::iterator  fieldDefListIter_t;

/*! 
    DataType_e identifiers are used within the runtime type 
    information struct inside each ProcModule
*/
enum DataType_e { 
    INVALID1 = -1, 
    EXPORTEND = 0, 
    LIST, 
    LISTEND, 
    CHAR, 
    INT8, 
    INT16, 
    INT32, 
    INT64,
    UINT8, 
    UINT16, 
    UINT32, 
    UINT64,
    STRING, 
    BINARY, 
    IPV4ADDR, 
    IPV6ADDR,
    FLOAT, 
    DOUBLE, 
    INVALID2 
};


/*! run time type information array */
typedef struct {
    enum DataType_e type;
    char *name;
} typeInfo_t;


// FIXME document!
const int MAX_FIELD_SET_SIZE = 16;

//! match/field types
typedef enum
{
    FT_EXACT =0,
    FT_RANGE,
    FT_SET,
    FT_WILD
} fieldType_t;

//! definition of a field
class field_t
{

public:
	string name;
	string type;
	fieldType_t mtype;
	unsigned short len;
	//! number of values
	unsigned short cnt;

	//! value definition:
	//! EXACT -> value[0]
	//! RANGE -> min in value[0], max in value[1]
	//! SET -> value[0-n] where value.len>0
	//! WILD -> no value
	vector<FieldValue> value;
	
	field_t(): mtype(FT_WILD), len(0), cnt(0)  {}
	
	~field_t();
	
	field_t(const field_t &param);
				
	void parseFieldValue(string val);
	
	string getInfo(void);
	
	bool operator==(const field_t &param);

	bool operator!=(const field_t &param);
	
   /**
    * Assignment operator. 
    */
   field_t &operator=(const field_t &other);
	
   void dump(ostream &os);
};

//! overload for <<, so that a field_t object can be thrown into an iostream
ostream& operator<< ( ostream &os, field_t &f );

//! field list (only push_back & sequential access)
typedef vector<field_t>            			fieldList_t;
typedef vector<field_t>::iterator  			fieldListIter_t;
typedef vector<field_t>::const_iterator  	fieldListconstIter_t;


} // namespace auction.

#endif // _FIELD_H_
