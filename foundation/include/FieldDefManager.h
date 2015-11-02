
/*!  \file   FieldDefManager.h

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
    Maintain field definitions and value definitions

    $Id: FieldDefManager.h 748 2015-10-28 15:51:00 amarentes $
*/

#ifndef _FIELD_DEF_MANAGER_H_
#define _FIELD_DEF_MANAGER_H_

#include "FieldDefParser.h"
#include "FieldValParser.h"

namespace auction
{

class FieldDefManager
{
	private:
		//! field definitions
		fieldDefList_t fieldDefs;

		//! field values
		fieldValList_t fieldVals;

		// name of field def and field vals files
		string fieldDefFileName, fieldValFileName;

	    //! load filter definitions
		void loadFieldDefs();

		//! load filter value definitions
		void loadFieldVals();
	
	public:
		
		//! Class constructor.
		FieldDefManager(string fdname, string fvname); 
		
		 //! destroy a FieldDefManager object
		~FieldDefManager();
		
		//! Get the field definitions 
		fieldDefList_t * getFieldDefs();
		
		//! Get the value definitions 
		fieldValList_t * getFieldVals();

};

}; // namespace auction

#endif // _FIELD_DEF_MANAGER_H_
