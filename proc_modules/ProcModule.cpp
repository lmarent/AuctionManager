
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

/*! \short   embed magic cookie number in every packet processing module
    _N_et_M_ate _P_rocessing Module */
int magic = PROC_MAGIC;

/*! \short   declaration of struct containing all function pointers of a module */
ProcModuleInterface_t func = 
{ 
    3, 
    initModule, 
    destroyModule, 
    execute, 
    getTimers, 
    destroy,
    reset, 
    timeout, 
    getModuleInfo, 
    getErrorMsg 
};


/*! \short   global state variable used within data export macros */
void *_dest;
int   _pos;
int   _listlen;
int   _listpos;


/* align macro */
#define ALIGN( var, type ) var = ( var + sizeof(type)-1 )/sizeof(type) * sizeof(type)

void initializeField(field_t *f)
{
	// Initialize the values for the field.
	for (int i=0 ; i < MAX_FIELD_SET_SIZE; i++)
	{
		FieldValue fielvalue;
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
        throw ProcError("Not a long number: %s", errp);
    }
    
    return n;
	
}

uint8_t parseUInt8(unsigned char *val)
{
	uint8_t val_return = val[0];
	return val_return;
}



unsigned long parseULong( string s )
{
    char *errp = NULL;
    unsigned long n;
    n = strtoul(s.c_str(), &errp, 0);

    if (s.empty() || (*errp != '\0')) {
        throw ProcError("Not an unsigned long number: %s", errp);
    }
    
    return n;	
}

uint16_t parseUInt16( unsigned char *val )
{
	uint16_t val_return;
	val_return = (uint16_t)val[0] << 8 | (uint16_t)val[1];
	return val_return;
}

long long parseLLong( string s )
{
    char *errp = NULL;
    long long n;
    n = strtoll(s.c_str(), &errp, 0);

    if (s.empty() || (*errp != '\0')) {
        throw ProcError("Not a long long number: %s", errp);
    }
    
    return n;	
}

uint32_t parseUInt32( unsigned char *val )
{
	uint32_t val_return;
	val_return = (uint32_t)val[0] << 24 |
				 (uint32_t)val[1] << 16 |
				 (uint32_t)val[2] << 8  |
				 (uint32_t)val[3];
	return val_return;
}

unsigned long long parseULLong( string s )
{
    char *errp = NULL;
    unsigned long long n;
    n = strtoull(s.c_str(), &errp, 0);

    if (s.empty() || (*errp != '\0')) {
        throw ProcError("Not an unsigned long long number: %s", errp);
    }
    
    return n;

}

int parseInt( string s )
{
    char *errp = NULL;
    int n;
    
    n = (int) strtol(s.c_str(), &errp, 0);
    if (s.empty() || (*errp != '\0'))
        throw ProcError("Not an int number: %s", errp);
    
    return n;
	
}

int parseBool(string s)
{
    if ((s == "yes") || (s == "1") || (s == "true")) {
        return 1;
    } else if ((s == "no") || (s == "0") || (s == "false")) {
        return 0;
    } else {
        throw ProcError("Invalid bool value: %s", s.c_str());
    }
	
}

float parseFloat(string s)
{
    char *errp = NULL;
    float n;

    n = strtof(s.c_str(), &errp);

    if (s.empty() || (*errp != '\0')) {
        throw ProcError("Not a float number: %s", errp);
    }

    return n;	
}

double parseDouble(string s)
{
    char *errp = NULL;
    double n;

    n = strtod(s.c_str(), &errp);

    if (s.empty() || (*errp != '\0')) {
        throw ProcError("Not a double number: %s", errp);
    }

    return n;

}




