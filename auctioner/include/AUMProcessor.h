
/*! \file   AUMProcessor.h

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
    manages and applies auction processing modules

    $Id: AuctionProcessor.h 748 2015-07-23 14:33:00Z amarentes $
*/

#ifndef _AUMPROCESSOR_H_
#define _AUMPROCESSOR_H_


#include "stdincpp.h"
#include "ProcModule.h"
#include "ModuleLoader.h"
#include "Bid.h"
#include "Allocation.h"
#include "AuctionManagerComponent.h"
#include "Error.h"
#include "Logger.h"
#include "EventScheduler.h"
#include "IpApMessageParser.h"

namespace auction
{

typedef enum
{
	AUM_SESSION_FIELD_SET_NAME = 0,
	AUM_REQUEST_FIELD_SET_NAME
} agentFieldSet_t;

typedef struct
{
    ProcModule *module;
    ProcModuleInterface_t *mapi; // module API
    // config params for module
    configParam_t *params;
} ppaction_t;


typedef struct {

    ppaction_t action; // action module data.
    Auction *auction; // auction to start execution.
    bidDB_t bids;  // Bids competing in the auction.
    
} auctionProcess_t;

//! action list for each auction
typedef map<int, auctionProcess_t>            auctionProcessList_t;
typedef map<int, auctionProcess_t>::iterator  auctionProcessListIter_t;

typedef map< agentFieldSet_t, set<ipap_field_key> >  		  setFieldsList_t;
typedef map< agentFieldSet_t, set<ipap_field_key> >::iterator  setFieldsListIter_t;


/*! \short   manage and execute algoirthms for a set of auctions.

    the AuctionProcessor class allows auctioning between bids 
*/

class AUMProcessor : public AuctionManagerComponent, public IpApMessageParser
{
  private:

    //! associated module loader 
    //! this is the algorithm to execute for the bid
    ModuleLoader *loader;

	//! field definitions
    fieldDefList_t fieldDefs;

    //! name of field defs file.
    string fieldDefFileName;
    
    //! load field definitions
    void loadFieldDefs(string fname);

    //! action of every auction being processed.
    auctionProcessList_t  auctions;

    //! add timer events to scheduler
    void addTimerEvents( int auctionID, int actID, ppaction_t &act, EventScheduler &es );

	miscList_t readMiscData( ipap_template *templ, ipap_data_record &record);
	
	bool intersects( time_t startDttmAuc, time_t stopDttmAuc, 
							time_t startDttmReq, time_t stopDttmReq );
	
	bool forResource(string resourceAuc, string resourceIdReq);
	
  public:

	static setFieldsList_t fieldSets;

    /*! \short   construct and initialize a PacketProcessor object

        \arg \c cnf        config manager
        \arg \c fdname     field definition file name
        \arg \c threaded   run as separate thread
        \arg \c moduleDir  action module directory
    */
    AUMProcessor(ConfigManager *cnf, string fdname, int threaded, string moduleDir = "" );

    //!   destroy a Auction Processor object, to be overloaded
    virtual ~AUMProcessor();

    //! add auctions
    virtual void addAuctions( auctionDB_t *auctions, EventScheduler *e );

    //! delete bids
    virtual void delAuctions( auctionDB_t *aucts );

    //! delete bids
    virtual void delBids( bidDB_t *bids );


    //! execute the algorithm
    int executeAuction(int rid, string rname);

    /*! \short   add a Bid to auction bid list
        \arg \c auctionSet    Auction set
        \arg \c auctionName   Auction name
        \arg \c b 			  Pointer to bid to insert
    */
    void addBidAuction(string auctionSet, string auctionName, Bid *b );


    /*! \short   add a Auction and its associated auction process list
        \arg \c a   pointer to auction
        \arg \c e   pointer to event scheduler (timer events)
        \returns 0 - on success, <0 - else
    */
    int addAuction( Auction *a, EventScheduler *e );


    /*! \short   delete a Bid to auction bid list
        \arg \c auctionSet    Auction set
        \arg \c auctionName   Auction name
        \arg \c b 			  Pointer to bid to delete
    */
    void delBidAuction( string auctionSet, string auctionName, Bid *b );


    /*! \short   delete an Auction from the auction process list
        \arg \c a  pointer to auction
        \returns 0 - on success, <0 - else
    */
    int delAuction( Auction *a );
	
    /*! \short   get the auctions applicable given the options within the message.
        \arg \message a  pointer to a message with the options to filter.
        \returns 0 - list of application auctions.
    */
	auctionDB_t * getApplicableAuctions(ipap_message *message);

    /*! \short   get the session information within the message.
        \arg \message a pointer to a message with the options to filter.
        \returns 0 - map of values that identify the session.
    */
	map<ipap_field_key,string> getSessionInformation(ipap_message *message);

    //! handle file descriptor event
    virtual int handleFDEvent(eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds);

    //! thread main function
    void main();
   
    //! get information about load module
    string getInfo();

    //! dump a AUMProcessor object
    void dump( ostream &os );
	
	fieldDefList_t * getFieldDefinitions() { return &fieldDefs; }

    //! get the number of action modules currently in use
    int numModules() 
    { 
        return loader->numModules(); 
    }

    // handle module timeouts
    void timeout(int rid, int actid, unsigned int tmID);

    //! get xml info for a specific module
    string getModuleInfoXML( string modname );

    virtual string getConfigGroup() 
    { 
        return "AUM_PROCESSOR"; 
    }

    virtual void waitUntilDone();

	static set<ipap_field_key> getSetField(agentFieldSet_t setName);

};


//! overload for <<, so that a Auction Processor object can be thrown into an iostream
ostream& operator<< ( ostream &os, AUMProcessor &pe );

}; // namespace auction

#endif // _AUMPROCESSOR_H_
