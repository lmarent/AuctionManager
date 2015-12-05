
/*! \file ProcModule.cpp

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombiay

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
    implementation of helper functions for Packet Processing Modules

    $Id: ProcModule.cpp 748 2015-08-03 13:48:00 amarentes $

*/

#include "ProcModule.h"
#include "ProcError.h"
#include <openssl/rand.h>


/*! \short   embed magic cookie number in every packet processing module
    _N_et_M_ate _P_rocessing Module */
int magic = PROC_MAGIC;

const string TIME_FORMAT      = "%Y-%m-%d %T";

/*! \short   declaration of struct containing all function pointers of a module */
auction::ProcModuleInterface_t func = 
{ 
    3, 
    auction::initModule, 
    auction::destroyModule, 
    auction::execute, 
    auction::execute_user, 
    auction::destroy,
    auction::reset, 
    auction::getModuleInfo, 
    auction::getErrorMsg 
};


/*! \short   global state variable used within data export macros */
void *_dest;
int   _pos;
int   _listlen;
int   _listpos;


/* align macro */
#define ALIGN( var, type ) var = ( var + sizeof(type)-1 )/sizeof(type) * sizeof(type)

void initializeField(auction::field_t *f)
{
	// Initialize the values for the field.
	for (int i=0 ; i < auction::MAX_FIELD_SET_SIZE; i++)
	{
		auction::FieldValue fielvalue;
		f->value.push_back(fielvalue);
	}

}


/*! Parse parameter functions */

long parseLong( string s )
{
    char *errp = NULL;
    long n;
    n = strtol(s.c_str(), &errp, 0);

    if (s.empty() || (*errp != '\0')) {
        throw auction::ProcError("Not a long number: %s", errp);
    }
    
    return n;
	
}

uint8_t parseUInt8(char *val)
{
	
	unsigned char uval[2];
	
	memcpy(uval, val, sizeof(char) + 1);
	
	uint8_t val_return = uval[0];
	
	return val_return;
}



unsigned long parseULong( string s )
{
    char *errp = NULL;
    unsigned long n;
    n = strtoul(s.c_str(), &errp, 0);

    if (s.empty() || (*errp != '\0')) {
        throw auction::ProcError("Not an unsigned long number: %s", errp);
    }
    
    return n;	
}

uint16_t parseUInt16( char *val )
{

	unsigned char uval[3];
	
	memcpy(uval, val, 2*sizeof(char) + 1);

	uint16_t val_return;
	val_return = (uint16_t)uval[0] << 8 | (uint16_t)uval[1];
	
	return val_return;
}

long long parseLLong( string s )
{
    char *errp = NULL;
    long long n;
    n = strtoll(s.c_str(), &errp, 0);

    if (s.empty() || (*errp != '\0')) {
        throw auction::ProcError("Not a long long number: %s", errp);
    }
    
    return n;	
}

uint32_t parseUInt32( char *val )
{

	char *errp = NULL;
    uint32_t n;
    n = strtoul(val, &errp, 0);

    if ((*errp != '\0')) {
        throw auction::ProcError("Not an unsigned long long number: %s", errp);
    }
    
    return n;
				 
}

unsigned long long parseULLong( string s )
{
    char *errp = NULL;
    unsigned long long n;
    n = strtoull(s.c_str(), &errp, 0);

    if (s.empty() || (*errp != '\0')) {
        throw auction::ProcError("Not an unsigned long long number: %s", errp);
    }
    
    return n;

}

int parseInt( string s )
{
    char *errp = NULL;
    int n;
    
    n = (int) strtol(s.c_str(), &errp, 0);
    if (s.empty() || (*errp != '\0'))
        throw auction::ProcError("Not an int number: %s", errp);
    
    return n;
	
}

int parseBool(string s)
{
    if ((s == "yes") || (s == "1") || (s == "true")) {
        return 1;
    } else if ((s == "no") || (s == "0") || (s == "false")) {
        return 0;
    } else {
        throw auction::ProcError("Invalid bool value: %s", s.c_str());
    }
	
}

float parseFloat(string s)
{
    char *errp = NULL;
    float n;

    n = strtof(s.c_str(), &errp);

    if (s.empty() || (*errp != '\0')) {
        throw auction::ProcError("Not a float number: %s", errp);
    }

    return n;	
}

double parseDouble(string s)
{
    char *errp = NULL;
    double n;

    n = strtod(s.c_str(), &errp);

    if (s.empty() || (*errp != '\0')) {
        throw auction::ProcError("Not a double number: %s", errp);
    }

    return n;

}


double getDoubleField(auction::fieldList_t *fields, string name)
{
	
	auction::fieldListIter_t field_iter;
		
	for (field_iter = fields->begin(); field_iter != fields->end(); ++field_iter )
	{
	
	    fprintf(stdout, "field:%s", (field_iter->name).c_str());
	
		if ((field_iter->name).compare(name) == 0 ){
			return parseDouble( ((field_iter->value)[0]).getValue());
		}
	}
	
	throw auction::ProcError(AUM_FIELD_NOT_FOUND_ERROR, 
					"Proc module - The given field was not included");
}

float getFloatField(auction::fieldList_t *fields, string name)
{
	
	auction::fieldListIter_t field_iter;
		
	for (field_iter = fields->begin(); field_iter != fields->end(); ++field_iter )
	{
	
		if ((field_iter->name).compare(name) == 0 ){
			return parseFloat( ((field_iter->value)[0]).getValue());
		}
	}
	
	throw auction::ProcError(AUM_FIELD_NOT_FOUND_ERROR, 
					"Proc module - The given field was not included");
}

/* ------------------------- parseTime ------------------------- */
time_t 
parseTime(string timestr)
{
    struct tm  t;
  
    if (timestr[0] == '+') {
        // relative time in secs to start
        try {
			struct tm tm;
            int secs = parseInt(timestr.substr(1,timestr.length()));
            time_t start = time(NULL) + secs;
            return mktime(localtime_r(&start,&tm));
        } catch (Error &e) {
            throw Error("Incorrect relative time value '%s'", timestr.c_str());
        }
    } else {
        // absolute time in the time format 
        if (timestr.empty()) {
            return 0;
        } else if (timestr.find_first_not_of("0123456789") == std::string::npos) {
			try{
				uint64_t secs = parseULong(timestr);
				time_t start = secs;
				return mktime(localtime_r(&start,&t));
			} catch (Error &e) {
				throw Error("Incorrect absolute time value '%s'", timestr.c_str());
			}
		}
        else if ((strptime(timestr.c_str(), TIME_FORMAT.c_str(), &t) == NULL)){
			return 0;
        }
    }
    return mktime(&t);
}



string getStringField(auction::fieldList_t *fields, string name)
{
	
	auction::fieldListIter_t field_iter;
		
	for (field_iter = fields->begin(); field_iter != fields->end(); ++field_iter )
	{
	
		if ((field_iter->name).compare(name) == 0 ){
			return ((field_iter->value)[0]).getValue();
		}
	}
	
	throw auction::ProcError(AUM_FIELD_NOT_FOUND_ERROR, 
					"Proc module - The given field was not included");
}

void fillField(auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals,
			   int eno, int ftype, string value, auction::fieldList_t *fields)
{
	auction::fieldDefItem_t iter;
	auction::field_t field;
	initializeField(&field);
	
	iter = auction::IpApMessageParser::findField(fieldDefs, eno, ftype);
	field.name = iter.name;
	field.len = iter.len;
	field.type = iter.type;
	auction::IpApMessageParser::parseFieldValue(fieldVals, value, &field);
	
	fields->push_back(field);

}

string doubleToString (double value)
{
	std::ostringstream s;
	s << value;
	return s.str();
}

string floatToString (float value)
{
	std::ostringstream s;
	s << value;
	return s.str();
}

string uint32ToString(uint32_t value)
{
	std::ostringstream s;
	s << value;
	return s.str();
}

string uint64ToString (uint64_t value)
{
	std::ostringstream s;
	s << value;
	return s.str();
}


string intToString (int value)
{
	std::ostringstream s;
	s << value;
	return s.str();
}

bool caseInsensitiveStringCompare( const std::string& str1, const std::string& str2 ) {
    std::string str1Cpy( str1 );
    std::string str2Cpy( str2 );
    std::transform( str1Cpy.begin(), str1Cpy.end(), str1Cpy.begin(), ::tolower );
    std::transform( str2Cpy.begin(), str2Cpy.end(), str2Cpy.begin(), ::tolower );
    return ( str1Cpy == str2Cpy );
}

uint32_t getId()
{

	uint32_t nbr;
	
	int ret = RAND_bytes((unsigned char *) &nbr, sizeof(uint32_t));

	assert( ret == 1 );
	
	return nbr;
}
