
/*! \file  proc_modules/ProcModule.h

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Measurement and Accounting System (NETAUM).

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
    declaration of interface for Auction Modules

    $Id: ProcModule.h 748 2015-08-03 13:47:00 amarentes $

*/


#ifndef __PROCMODULE_H
#define __PROCMODULE_H


#include "ProcModuleInterface.h"
#include "IpApMessageParser.h"
#include "Auction.h"
#include "Field.h"
#include "FieldValue.h"
#include "BiddingObject.h"
#include "IpAp_field_container.h"
#include "IpAp_template.h"


/*! Functions for reading parameters and filters, 
 *  It is assumed that data is already verified. */

typedef struct
{
	string bidSet;
	string bidName;
	string elementName;
	double quantity;
	double sellPrice;
} alloc_proc_t;

void initializeField(auction::field_t *f); 
 
long parseLong( string s );

uint8_t parseUInt8( char *val );

unsigned long parseULong( string s );

uint16_t parseUInt16( char *val );

long long parseLLong( string s );

uint32_t parseUInt32( char *val );

unsigned long long parseULLong( string s );

int parseInt( string s );

int parseBool( string s);

float parseFloat( string s);

double parseDouble( string s);

double getDoubleField(auction::fieldList_t *fields, string name);

float getFloatField(auction::fieldList_t *fields, string name);

time_t parseTime(string timestr);

void fillField(auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals,
			   int eno, int ftype, string value, auction::fieldList_t *fields);

string getStringField(auction::fieldList_t *fields, string name);

string doubleToString(double value);

string floatToString(float value);

string uint64ToString (uint64_t value);

string uint32ToString(uint32_t value);

string intToString(int value);

bool caseInsensitiveStringCompare( const std::string& str1, const std::string& str2 );

/*! \short   declaration of struct containing all function pointers of a module */
extern auction::ProcModuleInterface_t func;

#endif /* __PROCMODULE_H */

