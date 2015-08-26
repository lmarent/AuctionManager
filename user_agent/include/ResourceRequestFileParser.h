
/*!  \file   ResourceRequestFileParser.h

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Quality Manager System (NETAUM).

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
    parse resource request files
    Code based on Netmate Implementation

    $Id: ResourceRequestFileParser.h 748 2015-08-25 17:05:00 amarentes $
*/

#ifndef _RESOURCEREQUEST_FILEPARSER_H_
#define _RESOURCEREQUEST_FILEPARSER_H_


#include "stdincpp.h"
#include "libxml/parser.h"
#include "Logger.h"
#include "XMLParser.h"
#include "ProcModuleInterface.h"
#include "FieldDefParser.h"
#include "ConfigParser.h"
#include "ResourceRequestIdSource.h"
#include "ResourceRequest.h"


//! Allocation list
typedef vector<ResourceRequest*>            resourceRequestDB_t;
typedef vector<ResourceRequest*>::iterator  resourceRequestDBIter_t;

class ResourceRequestFileParser : public XMLParser
{
  private:

    Logger *log;
    int ch;

    //! parse a config item
    configItem_t parsePref(xmlNodePtr cur);

	//! parse a field value
	void parseFieldValue(string value, field_t *f);   
		
	//! Calculates intervals associated to auction.
	void calculateInterval(time_t now, miscList_t *miscList, 
							resourceReq_interval_t *resInterval);

    //! parse time string
    time_t parseTime(string timestr);

    //! get a value by name from the misc rule attriutes
    string getMiscVal(miscList_t *miscList, string name);    

  public:

    ResourceRequestFileParser( string fname );

    ResourceRequestFileParser( char *buf, int len );

    virtual ~ResourceRequestFileParser() {}

    //! parse resource request and add parsed resource requests.
    virtual void parse(fieldDefList_t *fields, 
					   resourceRequestDB_t *requests,
					   ResourceRequestIdSource *idSource );
};

#endif // _RESOURCEREQUEST_FILEPARSER_H_
