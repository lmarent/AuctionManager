
/*! \file   AgentManagerInfo.h

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

    $Id: CommandLineArgs.h 748 2015-07-26 10:07:00 amarentes $
*/

#ifndef _AGENT_MANAGER_INFO_H_
#define _AGENT_MANAGER_INFO_H_


#include <iostream>
#include "Error.h"

//! types of information available via get_info command
enum agentInfoType_t {
	AGI_UNKNOWN = -1,
    AGI_ALL = 0,
    AGI_AGENTMANAGER_VERSION, 
    AGI_UPTIME, 
    AGI_BIDS_STORED, 
    AGI_CONFIGFILE,  
    AGI_USE_SSL, 
    AGI_HELLO,
    AGI_BIDLIST,
    AGI_BID,
    // insert new items here
    AGI_NUMAGENTMANAGERINFOS
};

//! type and value of a single agent info
struct agentInfo_t
{
    agentInfoType_t type;
    string     param;
    
    agentInfo_t( agentInfoType_t t, string p = "" ) {
        type = t;
        param = p;
    }
};

//! list of agent infos (type + parameters)
typedef vector< agentInfo_t >            agentInfoList_t;
typedef vector< agentInfo_t >::iterator  agentInfoListIter_t;

//! get numeric info type from string
typedef std::map< string, agentInfoType_t >            agentTypeMap_t;
typedef std::map< string, agentInfoType_t >::iterator  agentTypeMapIter_t;


class AgentManagerInfo {
    
  private:
    
    static char *INFOS[];      //!< names of the info items
    static agentTypeMap_t typeMap; 

    //!< currently stored list of info items
    agentInfoList_t *list;  

public:

    AgentManagerInfo();

    ~AgentManagerInfo();

    //! get info name from info type
    static string getInfoString(agentInfoType_t info);

    //! get info type from info name
    static agentInfoType_t getInfoType (string item);

    //! add info by type
    void addInfo( agentInfoType_t type, string param = "");

    //! add info by name
    void addInfo( string item, string param = "");

    /*! \brief release list of added info_t items.
               caller is responsible for the disposal of this list
    */
    inline agentInfoList_t *getList()
    {
        agentInfoList_t *tmp = list;
        list = NULL;
        return tmp;
    }

};


#endif // _AGENT_MANAGER_INFO_H_
