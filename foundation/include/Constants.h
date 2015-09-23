
/* \file constants.h

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
	here all string and numeric constants for the netaum toolset are declared

	$Id: AuctionManagerComponent.h 748 2015-07-26 9:33:00 amarentes $

*/


#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include "config.h"

namespace auction
{

// Auction.h
extern const string DEFAULT_CONFIG_FILE;
extern const string NETAUM_LOCK_FILE;
extern const int    DEF_SNAPSIZE;

// RuleManager.cpp
extern const unsigned int  DONE_LIST_SIZE;
extern const string        FIELDVAL_FILE;
extern const string        FILTERDEF_FILE;


// Logger.h
extern const string DEFAULT_LOG_FILE;

// BidFileParser.cpp
extern const string BIDFILE_DTD;

// AuctionFileParser.cpp
extern const string AUCTIONFILE_DTD;


// Bid.cpp
extern const string TIME_FORMAT;
extern const string DEFAULT_SETNAME;

// FieldDefParser.cpp
extern const string FIELDDEF_FILE;
extern const string FIELDDEF_DTD;

// FilterValParser.cc
extern const string FIELDVAL_DTD;

// CtrlComm.cpp
extern const string REPLY_TEMPLATE; //!< html response template
extern const string MAIN_PAGE_FILE;
extern const string XSL_PAGE_FILE;
extern const int    EXPIRY_TIME;    //!< expiry time for web pages served from cache
extern const int    DEF_PORT;   //!< default TCP port to connect to

// ConfigParser.h
extern const string CONFIGFILE_DTD;

// narsh.cpp
extern const string PROMPT;    //!< user prompt for interactive mode
extern const string DEF_SERVER; //!< default netmate host  
extern const int    HISTLEN;   //!< history length (lines)
extern const string HISTFILE;  //!< name of the history file
extern const string DEF_XSL;    //!< xsl file for decoding xml responses
extern const string HOME;      //!< environment var pointing to the dir where the hist file is
extern const string HELP;      //!< help text in interactive shell

#ifdef USE_SSL
// certificate file location (SSL)
extern const string CERT_FILE;
#endif

};

#endif // _CONSTANTS_H_
