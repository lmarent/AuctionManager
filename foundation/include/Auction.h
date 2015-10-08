
/*!  \file   Auction.h

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
    Defines the auction object.
    Code based on Netmate Implementation

    $Id: Auction.h 748 2015-08-04 9:46:00 amarentes $
*/

#ifndef _AUCTION_H_
#define _AUCTION_H_

#include "stdincpp.h"
#include "Constants.h"
#include "Logger.h"
#include "AuctionTimer.h"
#include "ConfigParser.h"
#include "IpAp_template_container.h"
#include "IpAp_message.h"
#include "Field.h"

namespace auction
{


//! rule states during lifecycle
typedef enum
{
    AS_NEW = 0,
    AS_VALID,
    AS_SCHEDULED,
    AS_ACTIVE,
    AS_DONE,
    AS_ERROR
} AuctionState_t;

//! Algorith to execute for the auction process.
typedef struct
{
    string name;
    int defaultAct; //! 1 True, 0 False.
    configItemList_t conf;
} action_t;


typedef struct 
{
    interval_t interval;
    string procname;
} procdef_t;

typedef enum
{
	AS_BUILD_TEMPLATE = 0,
	AS_COPY_TEMPLATE
} AuctionTemplateMode_t;

typedef struct
{
	fieldDefItem_t field;
    ssize_t length;
	bool isBidtemplate;
	bool isOptBidTemplate;
	bool isAllocTemplate;
} auctionTemplateField_t;

//! action list (only push_back & sequential access)
typedef list<action_t>            actionList_t;
typedef list<action_t>::iterator  actionListIter_t;
typedef list<action_t>::const_iterator  actionListConstIter_t;

typedef map<string, auctionTemplateField_t>						auctionTemplateFieldList_t;
typedef map<string, auctionTemplateField_t>::iterator			auctionTemplateFieldListIter_t;
typedef map<string, auctionTemplateField_t>::const_iterator		auctionTemplateFieldLisConstIter_t;

class Auction
{
  private:

    Logger *log; //!< link to global logger object
    int ch;      //!< logging channel number used by objects of this class
    
    //! define the rules running time properties
    time_t start;
    time_t stop;

	//! define the execution intervals.
	interval_t	mainInterval;

    //! unique auctionID of this auction instance (has to be provided)
    int uid;

    //! state of this auction
    AuctionState_t state;

    //! name of the auction by convention this must be either: <name> or <resource>.<id>
    string auctionName;

    //! parts of auction name for efficiency
    string resource;
    string id;

    //! name of the auction set this auction belongs to
    string setName;
	
	//! Execution method to be called everytime that the auction is timeout.
	action_t action;

    //! list of misc stuff (start, stop, duration etc.)
    miscList_t miscList;

    /*! \short   parse identifier format 'sourcename.rulename'

        recognizes dor (.) in task identifier and saves sourcename and 
        rulename to the new malloced strings source and rname
    */
    void parseAuctionName(string rname);

    //! parse time string
    time_t parseTime(string timestr);

    //! get a value by name from the misc rule attriutes
    string getMiscVal(string name);

	//! Templates asociated with the auction. 
	ipap_template_container templates;

	//! Add a field to the template given as parameter.
	void addTemplateField(ipap_template *templ, 
		 			      ipap_field_container g_ipap_fields, 
						  int eno, int type);

	//! Add field keys to the template given as parameter.
	void addTemplateFieldKeys(ipap_template *templ, 
		  				      ipap_field_container g_ipap_fields);
	
	//! Calculate the number of fields to be included in the template type.
	int calculateNbrFieldTemplateData(ipap_templ_type_t templType, 
									  auctionTemplateFieldList_t &templFields);
	
	//! Create a template taking as input fields those in templFields.
	ipap_template * 
	createTemplate(auctionTemplateFieldList_t &templFields,
				   ipap_field_container g_ipap_fields,
				   ipap_templ_type_t templType);
	
	//! Build the templates related to the auction and store them in templateContainer
	void buildTemplates(auctionTemplateFieldList_t &templFields, 
							 ipap_template_container *templateContainer);
	
  public:
    
    void setState(AuctionState_t s) 
    { 
        state = s;
    }

    AuctionState_t getState()
    {
        return state;
    }

    int getUId() 
    { 
        return uid;
    }
    
    void setUId(int nuid)
    {
        uid = nuid;
    }
    
    string getSetName()
    {
        return setName;
    }
	
	void setSetName(string sname)
	{
		setName = sname;
	}
	
	void setAuctionName(string aname)
	{
		auctionName = aname;
	}
	
    string getAuctionName()
    {
        return auctionName;
    }
    
    string getAUctionID()
    {
		return id;
	}
    
    string getAuctionResource()
    {
        return resource;
    }
    
    time_t getStart()
    {
        return start;
    }
    
    time_t getStop()
    {
        return stop;
    }
    
    
    interval_t getInterval()
    {
        return mainInterval;
    }
     
            
    /*! \short   construct and initialize a Auction object
        \arg \c now   		current timestamp
        \arg \c sname   	auction set name
        \arg \c s  			aname  auction name
        \arg \c a  			action
        \arg \c m  			list of misc parameters
        \arg \c mode 		Mode for creating or using exting templates.
        \arg \c templFields	field list to be used in auction templates
        \arg \c message  	message where we include the new templates. 
        
    */
    Auction(time_t now, string sname, string aname, action_t &a, miscList_t &m, 
		    AuctionTemplateMode_t mode, auctionTemplateFieldList_t &templFields,
		    ipap_template_container *templates );

	/*! \short  construct an auction from another auction
	  	\arg \c rhs auction to copy from
	 */ 	
	Auction(const Auction &rhs);

    //! destroy a Auction object
    ~Auction();
   
    /*! \short   get names and values (parameters) of configured actions
        \returns a pointer (link) to an object that contains the configured action for this auction
    */
    action_t *getAction();

    /*! \short   get names and values (parameters) of misc. attributes

        \returns a pointer (link) to a ParameterSet object that contains the 
                 miscanellenous attributes of a configured auction
    */
    miscList_t *getMisc();


    /*! \short   get all templates added.

        \returns a pointer (link) to templates.
    */	
    ipap_template_container * getTemplateList(void);
    
    //! dump a Auction object
    void dump( ostream &os );

    //! get rule info string
    string getInfo(void);
	
};

//! overload for <<, so that a Auction object can be thrown into an iostream
ostream& operator<< ( ostream &os, Auction &ai );	

}; // namespace auction


#endif // _AUCTION_H_
