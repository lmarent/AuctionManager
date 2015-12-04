
/*! \file   AuctionManagerInfo.h

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
    Auction Manager info data structs used in getinfo auction command 
    Code based on Netmate

    $Id: CommandLineArgs.h 748 2015-07-26 10:07:00 amarentes $
*/

#ifndef _AUCTIONMANAGERINFO_H_
#define _AUCTIONMANAGERINFO_H_


#include <iostream>
#include "Error.h"

namespace auction
{

//! types of information available via get_info command
enum infoType_t {
    I_UNKNOWN = -1,
    I_ALL = 0,
    I_AUCTIONMANAGER_VERSION, 
    I_UPTIME, 
    I_BIDS_STORED, 
    I_CONFIGFILE,  
    I_USE_SSL, 
    I_HELLO,
    I_BIDLIST,
    I_BID,
    // insert new items here
    I_NUMAUCTIONMANAGERINFOS
};

//! type and value of a single meter info
struct info_t
{
    infoType_t type;
    string     param;
    
    info_t( infoType_t t, string p = "" ) {
        type = t;
        param = p;
    }
};

//! list of meter infos (type + parameters)
typedef vector< info_t >            infoList_t;
typedef vector< info_t >::iterator  infoListIter_t;

//! get numeric info type from string
typedef std::map< string, infoType_t >            typeMap_t;
typedef std::map< string, infoType_t >::iterator  typeMapIter_t;


class AuctionManagerInfo {
    
  private:
    
    static char *INFOS[];      //!< names of the info items
    static typeMap_t typeMap; 

    //!< currently stored list of info items
    infoList_t *list;  

public:

    AuctionManagerInfo();

    ~AuctionManagerInfo();

    //! get info name from info type
    static string getInfoString(infoType_t info);

    //! get info type from info name
    static infoType_t getInfoType (string item);

    //! add info by type
    void addInfo( infoType_t type, string param = "");

    //! add info by name
    void addInfo( string item, string param = "");

    /*! \brief release list of added info_t items.
               caller is responsible for the disposal of this list
    */
    inline infoList_t *getList()
    {
        infoList_t *tmp = list;
        list = NULL;
        return tmp;
    }

};

} // namespace auction

#endif // _AUCTIONMANAGERINFO_H_
