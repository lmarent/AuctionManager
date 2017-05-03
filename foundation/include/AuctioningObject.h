
/*!  \file   AuctioningObject.h

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
    Defines common classes and methods used for auctioning object.
    Code based on Netmate Implementation

    $Id: AuctioningObject.h 748 2015-10-30 8:53:00 amarentes $
*/

#ifndef _AUCTIONING_OBJECT_H_
#define _AUCTIONING_OBJECT_H_

#include "stdincpp.h"
#include "Logger.h"

namespace auction
{

//! defines states for all auctioning objects during lifecycle
typedef enum
{
    AO_NEW = 0,
    AO_VALID,
    AO_SCHEDULED,
    AO_ACTIVE,
    AO_DONE,
    AO_ERROR
} AuctioningObjectState_t;

extern const char *AuctionObjectStateNames[AO_ERROR+1];

class AuctioningObject
{

  protected:

    Logger *log; //!< link to global logger object
    int ch;      //!< logging channel number used by objects of this class

    //! unique auctionID of this auction instance (has to be provided)
    int uid;

    //! state of this auction
    AuctioningObjectState_t state;

    //! set this auctioning object belongs to
    string _set;

    //! name of the auctioning object by convention this must be either: <name> or <set>.name
    string _name;

	//! Parents' set 
	string _setParent;
	
	//! Parents' name
	string _nameParent;

  public:
    
    AuctioningObject(string channelName, string _set, string _name);
    
    AuctioningObject(string channelName, string _set, string _name, 
						string setParent, string nameParent);
    
    AuctioningObject(const AuctioningObject &rhs);
    
    virtual ~AuctioningObject();

    inline string getSet(){ return _set; }
	
	inline void setSet(string sname){ _set = sname; }
	
	inline void setName(string aname){ _name = aname; }
	
    inline string getName(){ return _name; }

    inline string getSetParent(){ return _setParent; }
    
    inline void setSetParent(string sParent){ _setParent = sParent; }
    
    inline string getNameParent(){ return _nameParent; }
    
    inline void setNameParent(string nameParent){ _nameParent = nameParent; }

    inline void setState(AuctioningObjectState_t s) { state = s; }

    inline AuctioningObjectState_t getState(){ return state; }

    inline int getUId(){ return uid; }
    
    inline void setUId(int nuid){ uid = nuid; }

	bool equals(const AuctioningObject &rhs);

	string getInfo(void);
};

}

#endif // _AUCTIONING_OBJECT_H_
