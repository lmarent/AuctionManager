
/*!  \file   FieldValue.h

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
	Class used to store field values with flexible length
	code based on NETMATE

    $Id: FieldValue.h 748 2015-07-23 15:30:00Z amarentes $

*/

#ifndef _FIELDVALUE_H_
#define _FIELDVALUE_H_

#include "stdinc.h"
#include "stdincpp.h"
#include "Error.h"

namespace auction
{

//! maximum length of a field value
const unsigned short MAX_FIELD_LEN = 32;

/* \short FieldValue

   This class stores field values in an array. If field values are
   set from numeric types like integers 
*/

class FieldValue
{
  private:

    string ftype;
	int len;						///< The length of the field value in characters
	string val;					///< value represented as string.
    
  public:

    /** Empty constructor.
		*/
    FieldValue() : len(0), ftype(), val() {}

    /** Constructor from type and value.
		*/
	FieldValue(string type, string value);
    
    /** Constructor from another field value.
		*/
    FieldValue(const FieldValue &param)
    {
		ftype = param.ftype;
		len = param.len;
		val = param.val;
	}
    
    /** Destructor.
		*/
    ~FieldValue() {} 
    
    /** Assignment operator. It equals a field value from another field value.
		*  @param  the field value to copy from.
		*/
	FieldValue& operator= (const FieldValue& param)
    {
		ftype = param.ftype;
		len = param.len;
		val = param.val;
		return *this;
	}
		
    
    /** Equal operator. verifies if the field given as parameter is equal
		*  @param  the field value to compare
		*/
	bool equal (const FieldValue& rhs, string type);
    
    /** Diferent operator. verifies if the field given as parameter is not equal
		*  @param  the field value to compare
		*/
	bool notEqual (const FieldValue& rhs, string type);

    /** 
     * This function is designed to be used in modules linked dynamically.
     * It is assumend that parameters type contains a valid type
     **/
    inline void setType(string type){ ftype = type;  }
    
    /** 
     * This function is designed to be used in modules linked dynamically
     * It is assumend that parameters value contains a valid value.
     **/
    inline void setValue(string value) { val = value; } 
		
    // get value as string
    string getString();

    // get value length
    int getLen()
    {
        return len;
    }

    // get access to the value
    string getValue()
    {
        return val;
    }

    //! get length by type (for all fixed types)
    static int getTypeLength(string type);

    string getType()
    {
		return ftype;
	}	
    
    string getInfo(void);
};

} // namespace auction

#endif // _FIELDVALUE_H_
