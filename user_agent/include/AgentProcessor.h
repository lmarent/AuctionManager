
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

namespace auction
{

typedef struct {

    Auction *auction; // auction to start execution.
    bidDB_t bids;  // Bids posted to the auction. 
    
} auctionProcess_t;

//! action list for each auction
typedef map<int, auctionProcess_t>            auctionProcessList_t;
typedef map<int, auctionProcess_t>::iterator  auctionProcessListIter_t;



/*! \short   manage and execute algorithms for an agent.

    the AgentProcessor class allows and agent to bid in auctions.
*/

class AgentProcessor : public AuctionManagerComponent
{
  private:

	//! field definitions
    fieldDefList_t fieldDefs;

    //! name of field defs file.
    string fieldDefFileName;
    
    //! load field definitions
    void loadFieldDefs(string fname);

    //! auction being processed.
    auctionProcessList_t  auctions;

    //! add timer events to scheduler
    void addTimerEvents( int auctionID, int actID, EventScheduler &es );

  public:

    /*! \short   construct and initialize a AgentProcessor object

        \arg \c cnf        config manager
        \arg \c fdname     field definition file name
        \arg \c threaded   run as separate thread
    */
    AgentProcessor(ConfigManager *cnf, string fdname, int threaded );

    //!   destroy a Agent Processor object, to be overloaded
    virtual ~AgentProcessor();

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
