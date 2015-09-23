
/*!  \file   FieldDefParser.h

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
    parse field definition files

    $Id: FieldDefParser.h 748 2015-07-23 17:00:00 amarentes $
*/

#ifndef _FIELD_DEFPARSER_H_
#define _FIELD_DEFPARSER_H_


#include "stdincpp.h"
#include "libxml/parser.h"
#include "Logger.h"
#include "XMLParser.h"
#include "Field.h"

namespace auction
{
	
class FieldDefParser : public XMLParser
{

  private:

    Logger *log;
    int ch;

    //! parse filter definition
    fieldDefItem_t parseDef(xmlNodePtr cur);

  public:

    FieldDefParser(string fname = "");

    virtual ~FieldDefParser() {}
		
    //! add parsed field definitions to list
    virtual void parse(fieldDefList_t *list);
};

}; // namespace auction

#endif // _FIELD_DEFPARSER_H_
