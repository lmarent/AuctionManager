
/*!  \file   FieldDefManager.cpp

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

    $Id: FieldDefManager.cpp 748 2015-10-28 15:51:00 amarentes $
*/

#include "FieldDefManager.h"
#include "Constants.h"


using namespace auction;

/* -------------------- isReadableFile -------------------- */

static int isReadableFile( string fileName ) {

    FILE *fp = fopen(fileName.c_str(), "r");

    if (fp != NULL) {
        fclose(fp);
        return 1;
    } else {
        return 0;
    }
}

//! Class constructor.
FieldDefManager::FieldDefManager(string fdname, string fvname):
fieldDefFileName(fdname), fieldValFileName(fvname)
{

	// load the field def list
    loadFieldDefs();
	
    // load the field val list
    loadFieldVals();

}
		
//! destroy a FieldDefManager object
FieldDefManager::~FieldDefManager()
{
 // Nothing to do.
}


//! load filter definitions
void 
FieldDefManager::loadFieldDefs()
{
	string fname;
	
    if (fieldDefFileName.empty()) {
        if (fname.empty()) {
            fname = FIELDDEF_FILE;
		}
    } else {
        fname = fieldDefFileName;
    }

    if (isReadableFile(fname)) {
        if (fieldDefs.empty() && !fname.empty()) {
            FieldDefParser f = FieldDefParser(fname.c_str());
            f.parse(&fieldDefs);
        }
    
    } else {
		throw  Error("file %s is not readable", fname.c_str());
    }

}

//! load filter value definitions
void 
FieldDefManager::loadFieldVals()
{
	string fname;
	
    if (fieldValFileName.empty()) {
        if (fname.empty()) {
            fname = FIELDVAL_FILE;
        }
    } else {
        fname = fieldValFileName;
    }

    if (isReadableFile(fname)) {
        if (fieldVals.empty() && !fname.empty()) {
            FieldValParser f = FieldValParser(fname.c_str());
            f.parse(&fieldVals);
        }
    } else {
		throw  Error("file %s is not readable", fname.c_str());
	}

}
			
//! Get the field definitions 
fieldDefList_t * 
FieldDefManager::getFieldDefs()
{
	return &fieldDefs;
}
		
//! Get the value definitions 
fieldValList_t * 
FieldDefManager::getFieldVals()
{
	return &fieldVals; 
}

