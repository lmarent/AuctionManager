
/* \file constants.cpp

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
	here all string and numeric constants for the NETAUM toolset are stored

	$Id: AuctionManagerComponent.h 748 2015-07-26 9:33:00 amarentes $

*/

#include "stdincpp.h"
#include "Constants.h"

using namespace std;


// Auction.h
const string DEFAULT_CONFIG_FILE = DEF_SYSCONFDIR "/netaum.conf.xml";
const string NETAUM_LOCK_FILE   = DEF_STATEDIR "/run/netaum.pid";
const int    DEF_SNAPSIZE        = 65536;

// BidManager.cpp
const unsigned int  DONE_LIST_SIZE = 50;
const string        FIELDVAL_FILE = DEF_SYSCONFDIR "/fieldval.xml";
const string        FIELDDEF_FILE = DEF_SYSCONFDIR "/fielddef.xml";

// Logger.h
const string DEFAULT_LOG_FILE = DEF_STATEDIR "/log/netaum.log";

// BidFileParser.cpp
const string BIDFILE_DTD     = DEF_SYSCONFDIR "/bidfile.dtd";

// Bid.cpp
const string TIME_FORMAT      = "%Y-%m-%d %T";
const string DEFAULT_SETNAME  = "default";

// FieldDefParser.cpp
const string FIELDDEF_DTD   = DEF_SYSCONFDIR "/fielddef.dtd";

// FieldValParser.cpp
const string FIELDVAL_DTD   = DEF_SYSCONFDIR "/fieldval.dtd";

// CtrlComm.cpp
const string REPLY_TEMPLATE  = DEF_SYSCONFDIR "/reply.xml";   //!< html response template
const string MAIN_PAGE_FILE  = DEF_SYSCONFDIR "/main.html";
const string XSL_PAGE_FILE   = DEF_SYSCONFDIR "/reply.xsl";
const int    EXPIRY_TIME     = 3600; 		  //!< expiry time for web pages served from cache
const int    DEF_PORT        = 12246;         //!< default TCP port to connect to

// ConfigParser.h
const string CONFIGFILE_DTD  = DEF_SYSCONFDIR "/netaum.conf.dtd";

// narsh.cpp
const string PROMPT     = "> ";          				//!< user prompt for interactive mode
const string DEF_SERVER = "localhost";   				//!< default netaum host  
const int    HISTLEN    = 200;           				//!< history length (lines)
const string HISTFILE   = "/.narsh_hist"; 				//!< name of the history file
const string DEF_XSL    = DEF_SYSCONFDIR "/reply2.xsl"; //!< xsl file for decoding xml responses

// environment var which points to the home dir where the hist file is
const string HOME = "HOME";

#ifdef USE_SSL
// certificate file location (SSL)
const string CERT_FILE = DEF_SYSCONFDIR "/netaum.pem";
#endif

// help text in interactive shell
const string HELP =  "" \
"interactive commands: \n" \
"help                           displays this help text \n" \
"quit, exit, bye                end telly program \n" \
"get_info <info_type> <param>   get auction information \n" \
"rm_bid <bid_id>               	remove bid(s) \n" \
"add_bid <bid>                 	add bid \n" \
"add_bids <bid_file>           	add bids from file \n" \
"get_modinfo <mod_name>         get auction module information \n \n" \
"any other text is sent to the server and the reply from the server is displayed.";

