
/*!  \file   ParserFcts.h

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
    functions for parsing string data into varies data formats
    code based on NETMATE

    $Id: ParserFcts.h 748 2015-07-23 15:30:00Z amarentes $
*/

#ifndef _PARSERFCTS_H_
#define _PARSERFCTS_H_

#include "config.h"
#include "stdincpp.h"
#include "Constants.h"

namespace auction
{

class ParserFcts
{
  public:

    static long parseLong(string s, long min=LONG_MIN, long max=LONG_MAX);

    static unsigned long parseULong(string s, unsigned long min=0, unsigned long max=ULONG_MAX);

    static long long parseLLong(string s, long long min=LLONG_MIN, long long max=LLONG_MAX);

    static unsigned long long parseULLong(string s, unsigned long long min=0, 
                                          unsigned long long max=ULLONG_MAX);

    static int parseInt(string s, int min=LONG_MIN, int max=LONG_MAX);

    static struct in_addr parseIPAddr(string s);

    static struct in6_addr parseIP6Addr(string s);

    static int parseBool(string s);

    static float parseFloat(string s, float min=MINFLOAT, float max=MAXFLOAT);

    static double parseDouble(string s, double min=MINDOUBLE, double max=MAXDOUBLE);

    static void parseItem(string type, string value);

};

}; // namespace auction


#endif // _PARSERFCTS_H_
