
/*! \file AgentManagerInfo.cpp

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

    $Id: AgentManagerInfo.cpp 748 2015-08-25 08:02:00 amarentes $
*/

#include "AgentManagerInfo.h"


char *AgentManagerInfo::INFOS[] = { "all", 
                             "version",
                             "uptime",
                             "bids_stored",
                             "configfile",
                             "use_ssl",
                             "hello",
                             "bidlist",
                             "bid" };

agentTypeMap_t AgentManagerInfo::typeMap; //std::map< string, agentInfoType_t >();


AgentManagerInfo::AgentManagerInfo() : list(NULL)
{
    // init static type map for name->type lookup via getInfoType()

    if (typeMap.empty()) {  // only fill map first time here
        for (int i = 0; i < AGI_NUMAGENTMANAGERINFOS; i++ ) {
            typeMap[INFOS[i]] = (agentInfoType_t)i;
        } 
    }
}


AgentManagerInfo::~AgentManagerInfo()
{
    if (list != NULL) {
        saveDelete(list);
    }
}


string AgentManagerInfo::getInfoString( agentInfoType_t info )
{
    if (info < 0 || info >= AGI_NUMAGENTMANAGERINFOS ) {
        return "";
    } else {
        return INFOS[info];
    }
}


agentInfoType_t AgentManagerInfo::getInfoType( string item )
{
    agentTypeMapIter_t i = typeMap.find(item);

    if (i != typeMap.end()) {  // item found
        return i->second; /* type */
    } else {
        return AGI_UNKNOWN;
    }
}


void AgentManagerInfo::addInfo( agentInfoType_t type, string param )
{
    if (list == NULL) {
        list = new agentInfoList_t();
    }

    list->push_back(agentInfo_t(type, param));
}


void AgentManagerInfo::addInfo( string item, string param )
{
    agentInfoType_t type = getInfoType(item);

    if (type == AGI_UNKNOWN) {  // no such info item
        throw Error("Unknown meter info '%s'", item.c_str());
    }

    switch (type) {
    case AGI_ALL:
        addInfo(AGI_HELLO);
        addInfo(AGI_UPTIME);
        addInfo(AGI_BIDS_STORED);
        addInfo(AGI_CONFIGFILE);
        addInfo(AGI_USE_SSL);
        addInfo(AGI_BIDLIST);
        break;
    case AGI_BID:
        addInfo(AGI_BID, param );
        break;
    default: 
        addInfo( type );
        break;
    }

}
