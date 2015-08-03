
/*! \file   Bid.h

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
    Bids in the system - All concrete bids inherit from this class.

    $Id: Bid.h 748 2015-07-23 15:30:00Z amarentes $
*/

#ifndef _BID_H_
#define _BID_H_

#include "stdincpp.h"
#include "Logger.h"
#include "ConfigParser.h"
#include "ProcModuleInterface.h"

//! Bid's states during lifecycle
typedef enum
{
    BS_NEW = 0,
    BS_VALID,
    BS_SCHEDULED,
    BS_ACTIVE,
    BS_DONE,
    BS_ERROR
} bidState_t;

typedef struct
{
    string name;
    fieldList_t fields;
} element_t;

//! element list (only push_back & sequential access)
typedef list<element_t>            elementList_t;
typedef list<element_t>::iterator  elementListIter_t;
typedef list<element_t>::const_iterator  elementListConstIter_t;



class Bid
{
private:
    Logger *log; //!< link to global logger object
    int ch;      //!< logging channel number used by objects of this class

public:

	Bid(int id, string sname, string rname, elementList_t &e);

	Bid(int id, const Bid &rhs);

	~Bid();
		
    void setState(bidState_t s) 
    { 
        state = s;
    }

    bidState_t getState()
    {
        return state;
    }

	
    time_t getStartTime()
    {
        return startTime;
    }
    
    time_t getEndTime()
    {
        return endTime;
    }

    int getUId() 
    { 
        return uid;
    }

    void setSetName(string _setName)
	{
		setName = _setName;
	}	

    string getSetName()
    {
        return setName;
    }

	void setBidName(string _bidName)
	{
		bidName = _bidName;
	}

    string getBidName()
    {
        return bidName;
    }


	string getInfo();
	
	string toString(element_t &elem);
	
    //! parse Start time string
    time_t parseStartTime(string timestr);
	
    //! parse End time string
    time_t parseEndTime(string timestr);


    /*! \short   get names and values (parameters) of configured elements
        \returns a pointer (link) to a list that contains the configured elements for this bid
    */
    elementList_t *getElements();

	
protected:
	
    //! unique bidID of this Rule instance (has to be provided)
    int uid;

    //! name of the rule for the external system calling the Auction Manager
    string bidName;

    //! name of the agent set this bid belongs to
    string setName;
    
    //! start time 
    time_t startTime;

    //! end time 
    time_t endTime;

	//! state of this rule
    bidState_t state;

    //! list of elements
    elementList_t elementList;

};

#endif // _BID_H_
