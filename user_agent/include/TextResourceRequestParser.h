
/*!  \file   TextResourceRequestParser.h 

    Copyright 2014-2015 Universidad de los Andes, Bogota, Colombia

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
    parser for API text resource request syntax

    $Id: TextResourceRequestParser.h 748 2015-08-26 08:12:00Z amarentes $
*/

#ifndef _MAPI_TEXT_RESOURCE_REQUEST_PARSER_H_
#define _MAPI_TEXT_RESOURCE_REQUEST_PARSER_H_


#include "stdincpp.h"
#include "ResourceRequestFileParser.h"

namespace auction
{

//! parser for API text Resource Request syntax

class TextResourceRequestParser
{

  private:

    Logger *log;
    int ch;
    char *buf;
    int len;
    string fileName;

    //! parse the value given as a value corresponding to the type 
    //! assigned in field f.
    void parseFieldValue(string value, field_t *f);
    
	//! Calculates intervals associated to resource request.
	void calculateInterval(time_t now, miscList_t *miscList, 
							resourceReq_interval_t *resInterval);

    //! parse time string
    time_t parseTime(string timestr);

    //! get a value by name from the misc attributes
    string getMiscVal(miscList_t *miscList, string name);    


  public:

    TextResourceRequestParser(string fname);

    TextResourceRequestParser(char *b, int l);

    virtual ~TextResourceRequestParser() {}

    //! parse given resource requests and add those parsed to requests
    virtual void parse(fieldDefList_t *filters, 
					   auctioningObjectDB_t *requests );

};

} // namespace auction

#endif // _MAPI_TEXT_RESOURCE_REQUEST_PARSER_H_
