
/*! \file   AgentProcessor.h

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
    manages and applies agent processing functions

    $Id: AgentProcessor.h 748 2015-08-25 11:17:00Z amarentes $
*/

#ifndef _AGENT_PROCESSOR_H_
#define _AGENT_PROCESSOR_H_


#include "stdincpp.h"
#include "Bid.h"
#include "Allocation.h"
#include "AuctionManagerComponent.h"
#include "Error.h"
#include "Logger.h"
#include "EventScheduler.h"
#include "ProcModule.h"
#include "ModuleLoader.h"
#include "FieldDefManager.h"

namespace auction
{

typedef struct {

    int index;
    string moduleName;			 // Name of the module;
    ProcModule *module; 		 // Module to execute
    ProcModuleInterface_t *mapi; // Module API
    fieldList_t *parameters; 	 // Parameters for execution
    auctionDB_t *auctions;  		 // Auctions to execute 
    
} requestProcess_t;

//! action list for each auction
typedef multimap<int, auctionProcess_t>            auctionProcessList_t;
typedef multimap<int, auctionProcess_t>::iterator  auctionProcessListIter_t;



/*! \short   manage and execute algorithms for an agent.

    the AgentProcessor class allows and agent to bid in auctions.
*/

class AgentProcessor : public AuctionManagerComponent, public FieldDefManager
{
  private:

    //! associated module loader 
    //! these are the algorithms to create bids for the user.
    ModuleLoader *loader;
    
    //! requests being processed.
    requestProcessList_t  requests;

    //! add timer events to scheduler
    void addTimerEvents( int auctionID, int actID, EventScheduler &es );

    //! pool of unique request process 
    IdSource idSource;

  public:

    /*! \short   construct and initialize a AgentProcessor object

        \arg \c cnf        config manager
        \arg \c fdname     field definition file name
        \arg \c fvname     field value definition file name
        \arg \c threaded   run as separate thread
    */
    AgentProcessor(ConfigManager *cnf, string fdname, string fvname, int threaded, string moduleDir = "" );

    //!   destroy a Agent Processor object, to be overloaded
    virtual ~AgentProcessor();

    //! Add a new request to the list of request to execute.
    int addRequest( fieldList_t *parameters, auctionDB_t *auctions, EventScheduler *e );

    //! delete request
    void delRequest( int index );

    //! execute the algorithm
    int executeRequest( int index );

    //! delete auctions
    void delAuctions( int index,  auctionsDB_t *auctions );

    /*! \short  add a Auction to auction  list
     *  \arg \c index 		  Id for the request.
        \arg \c a 			  Pointer to auction to insert
    */
    void addAuction(int index, Auction *a );


    /*! \short   delete an Auction from the request process list
        \arg \c a  pointer to auction
        \returns 0 - on success, <0 - else
    */
    void delAuction( int index,  Auction *a );


    //! handle file descriptor event
    virtual int handleFDEvent(eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds);

    //! thread main function
    void main();
   
    //! get information about load module
    string getInfo();

    //! dump a AUMProcessor object
    void dump( ostream &os );

    // handle module timeouts
    void timeout(int rid, int actid, unsigned int tmID);

    virtual string getConfigGroup() 
    { 
        return "AGENT_PROCESSOR"; 
    }

    virtual void waitUntilDone();

};


//! overload for <<, so that a Agent Processor object can be thrown into an iostream
ostream& operator<< ( ostream &os, AgentProcessor &pe );

}; // namespace auction

#endif // _AGENT_PROCESSOR_H_
