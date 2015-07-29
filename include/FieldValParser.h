
/*!  \file   FieldValParser.h

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
    parse field constant files
   
    $Id: FieldValParser.h 748 2015-07-23 14:33:00Z amarentes $
*/

#ifndef _FIELD_VALPARSER_H_
#define _FIELD_VALPARSER_H_


#include "stdincpp.h"
#include "libxml/parser.h"
#include "Logger.h"
#include "XMLParser.h"
#include "FieldValue.h"
#include "FieldDefParser.h"


//! filter value
typedef struct
{
    string name;
    string type;
    string svalue;
} fieldValItem_t;

//! field value list
typedef map<string,fieldValItem_t>            fieldValList_t;
typedef map<string,fieldValItem_t>::iterator  fieldValListIter_t;


class FieldValParser : public XMLParser
{
  private:

    Logger *log;
    int ch;

    //! parse field value definition
    fieldValItem_t parseDef(xmlNodePtr cur);

  public:

    FieldValParser(string fname);

    virtual ~FieldValParser() {}

    //! add parsed field values to list
    virtual void parse(fieldValList_t *list);
};


#endif // _FIELD_VALPARSER_H_
