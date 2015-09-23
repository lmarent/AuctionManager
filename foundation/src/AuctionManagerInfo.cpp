
/*! \file AuctionManagerInfo.cpp

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
    Auction Manager info data structs used in getinfo auction command 
    Code based on Netmate

    $Id: AuctionManagerInfo.cpp 748 2015-07-26 10:07:00 amarentes $
*/

#include "AuctionManagerInfo.h"

using namespace auction;

char *AuctionManagerInfo::INFOS[] = { "all", 
                             "version",
                             "uptime",
                             "bids_stored",
                             "configfile",
                             "use_ssl",
                             "hello",
                             "bidlist",
                             "bid" };

typeMap_t AuctionManagerInfo::typeMap; //std::map< string, infoType_t >();


AuctionManagerInfo::AuctionManagerInfo() : list(NULL)
{
    // init static type map for name->type lookup via getInfoType()

    if (typeMap.empty()) {  // only fill map first time here
        for (int i = 0; i < I_NUMAUCTIONMANAGERINFOS; i++ ) {
            typeMap[INFOS[i]] = (infoType_t)i;
        } 
    }
}


AuctionManagerInfo::~AuctionManagerInfo()
{
    if (list != NULL) {
        saveDelete(list);
    }
}


string AuctionManagerInfo::getInfoString( infoType_t info )
{
    if (info < 0 || info >= I_NUMAUCTIONMANAGERINFOS ) {
        return "";
    } else {
        return INFOS[info];
    }
}


infoType_t AuctionManagerInfo::getInfoType( string item )
{
    typeMapIter_t i = typeMap.find(item);

    if (i != typeMap.end()) {  // item found
        return i->second; /* type */
    } else {
        return I_UNKNOWN;
    }
}


void AuctionManagerInfo::addInfo( infoType_t type, string param )
{
    if (list == NULL) {
        list = new infoList_t();
    }

    list->push_back(info_t(type, param));
}


void AuctionManagerInfo::addInfo( string item, string param )
{
    infoType_t type = getInfoType(item);

    if (type == I_UNKNOWN) {  // no such info item
        throw Error("Unknown meter info '%s'", item.c_str());
    }

    switch (type) {
    case I_ALL:
        addInfo(I_HELLO);
        addInfo(I_UPTIME);
        addInfo(I_BIDS_STORED);
        addInfo(I_CONFIGFILE);
        addInfo(I_USE_SSL);
        addInfo(I_BIDLIST);
        break;
    case I_BID:
        addInfo(I_BID, param );
        break;
    default: 
        addInfo( type );
        break;
    }

}
