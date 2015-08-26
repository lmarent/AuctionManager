
/*! \file constants_agent.h

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
	here all string and numeric constants for the netagent toolset are declared

	$Id: constants_agent.h 748 2015-08-25 8:24:00 amarentes $

*/

#include "config.h"


#ifndef _CONSTANTS_AGENT_H_
#define _CONSTANTS_AGENT_H_


// Agent.h
extern const string NETAGENT_DEFAULT_CONFIG_FILE;

// Logger.h
extern const string AGENT_DEFAULT_LOG_FILE;

// ConfigParser.h
extern const string AUM_CONFIGFILE_DTD;

#ifdef USE_SSL
// certificate file location (SSL)
extern const string AUM_CERT_FILE;
#endif

// Agent.h
extern const string NETAGENT_LOCK_FILE;

// ResourceRequestFileParser.cpp
extern const string RESOURCE_FILE_DTD;

#endif // _CONSTANTS_AGENT_H_
