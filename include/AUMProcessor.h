
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
#include "AuctionManagerComponent.h"
#include "Error.h"
#include "Logger.h"
#include "EventScheduler.h"


typedef struct
{
    ProcModule *module;
    ProcModuleInterface_t *mapi; // module API
    configParam_t *params;
} ppaction_t;



typedef struct {

    Bid *bid;

} bidActions_t;

//! action list for each rule
typedef vector<bidActions_t>            bidActionList_t;
typedef vector<bidActions_t>::iterator  bidActionListIter_t;


/*! \short   manage and apply auctions for a set of bids.

    the AuctionProcessor class allows to auction between bids 
*/

class AUMProcessor : public AuctionManagerComponent
{
  private:

    //! number of bids
    int numBids;

	//! Algorithm to execute the bidding process.
	ppaction_t algorithm;

    //! associated module loader 
    //! this is the algorithm to execute for the bid
    ModuleLoader *loader;

    //! action list for bids
    bidActionList_t  bids;

    //! add timer events to scheduler
    void addTimerEvents( int bidID, int actID, ppaction_t &act, EventScheduler &es );

  public:

    /*! \short   construct and initialize a PacketProcessor object

        \arg \c cnf        config manager
        \arg \c threaded   run as separate thread
        \arg \c moduleDir  action module directory
    */
    AUMProcessor(ConfigManager *cnf, int threaded, string moduleDir = "" );

    //!   destroy a Auction Processor object, to be overloaded
    virtual ~AUMProcessor();

    //! add bids
    virtual void addBids( bidDB_t *bids );

    //! delete bids
    virtual void delBids( bidDB_t *bids );

    //! execute the algorithm
    int execute(EventScheduler *e );

    //! handle file descriptor event
    virtual int handleFDEvent(eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds);

    //! thread main function
    void main();

    /*! \short return -1 (no packet seen), 0 (timeout), >0 (no timeout; adjust last time)

        \arg \c bidId  - number indicating matching bid for packet
    */
    unsigned long bidTimeout(int bidID, unsigned long ival, time_t now);
    
    //! get information about loaded modules
    string getInfo();

    //! dump a AUMProcessor object
    void dump( ostream &os );

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

};


//! overload for <<, so that a Auction Processor object can be thrown into an iostream
ostream& operator<< ( ostream &os, AUMProcessor &pe );


#endif // _AUMPROCESSOR_H_
