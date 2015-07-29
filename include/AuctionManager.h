
/*! \file   AuctionManager.h

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Auction Manager System (NETAUM).

    NETAUM is free software; you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    NETQoS is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    Code based on Netmate
 
    Description:
    the main class 

    $Id: AuctionManager.h 748 2015-07-23 14:33:00Z amarentes $
*/

#ifndef _AuctionManager_H_
#define _AuctionManager_H_


#include <iostream>
#include "Error.h"
#include "Logger.h"
#include "CommandLineArgs.h"
#include "ConfigManager.h"
#include "BidManager.h"
#include "CtrlComm.h"
#include "EventScheduler.h"
#include "Constants.h"
#include "AuctionManagerComponent.h"
#include "AUMProcessor.h"

/*! \short   Auction Manager class description
  
    detailed Auction Manager class description
*/

class AuctionManager
{
  public:

    // FIXME document!
    static int s_sigpipe[2];
 
  private:
    
    //!< start time of the Auction Manager
    time_t startTime;

    //! config file name
    string configFileName;

    //! log file name
    string logFileName;

    auto_ptr<Logger>          	log;
    auto_ptr<CommandLineArgs> 	args;
    auto_ptr<PerfTimer>       	perf;
    auto_ptr<ConfigManager>   	conf;
    auto_ptr<RuleManager>     	rulm;
    auto_ptr<EventScheduler>  	evnt;

    auto_ptr<AuctionProcessor> 	proc;    
    auto_ptr<CtrlComm>        	comm;

    //! logging channel number used by objects of this class
    int ch;

     // FD list (from QualityManagerComponent.h)
    fdList_t fdList;

    // 1 if the procedure for applying executing auctions runs in a separate thread
    int pprocThread;

    // 1 if remote control interface is enabled
    static int enableCtrl;

    // signal handlers
    static void sigint_handler(int i);
    static void sigusr1_handler(int i);
    static void sigalarm_handler(int i);

    // exit function called on exit
    static void exit_fct(void);

    //! return 1 if a Auction Manager is already running on the host
    int alreadyRunning();

    //! get info string for get_info response
    string getInfo(infoType_t what, string param);

    //! hello message for the users.
    string getHelloMsg();

    //! execution information and version.
    string getAuctionManagerInfo(infoList_t *i);

  public:

    /*! \short   construct and initialize a Auction Manager object

        detailed constructor description for Auction server

        \arg \c argc - number of command line arguments
        \arg \c argv - list of command line arguments
    */
    AuctionManager(int argc, char *argv[]);


    /*! \short   destroy a Quality Manager object
        detailed destructor description for Quality Manager
    */
    ~AuctionManager();

    void run();

    //! dump a Auction Manager object
    void dump( ostream &os );

    //! handle the events
    void handleEvent(Event *e, fd_sets_t *fds);

};


//! overload for <<, so that a Auction Manager object can be thrown into an ostream
std::ostream& operator<< ( std::ostream &os, AuctionManager &obj );


#endif // _AuctionManager_H_
