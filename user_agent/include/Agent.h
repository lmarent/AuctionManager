
/*! \file   Agent.h

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
    Code based on Netmate
 
    Description:
    the agent class that establishes what an agent in our system can make.

    $Id: Agent.h 748 2015-08-25 08:17:00Z amarentes $
*/

#ifndef _AGENT_H_
#define _AGENT_H_


#include <iostream>
#include "Error.h"
#include "Logger.h"
#include "CommandLineArgs.h"
#include "ConfigManager.h"
#include "BidManager.h"
#include "AuctionManager.h"
#include "CtrlComm.h"
#include "EventSchedulerAgent.h"
#include "Constants.h"
#include "AuctionManagerComponent.h"
#include "AgentProcessor.h"

namespace auction
{

/*! \short   Agent class description
  
    detailed Agent class description
*/

class Agent
{
  public:

    // FIXME document!
    static int s_sigpipe[2];
 
  protected:
    
    //!< start time of the Agent
    time_t startTime;

    //! config file name
    string configFileName;

    //! log file name
    string logFileName;

    auto_ptr<Logger>          			log;
    auto_ptr<CommandLineArgs> 			args;
    auto_ptr<AuctionTimer>      		auct;
    auto_ptr<ConfigManager>   			conf;
    auto_ptr<BidManager>     			bidm;
    auto_ptr<AuctionManager>   			aucm;
    auto_ptr<EventSchedulerAgent>  		evnt;

    auto_ptr<AgentProcessor> 			proc;    
    auto_ptr<CtrlComm>        			comm;

    //! logging channel number used by objects of this class
    int ch;

     // FD list (from AuctionManagerComponent.h)
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

    //! return 1 if a Agent Manager is already running on the host
    int alreadyRunning();

    //! get info string for get_info response
    string getInfo(agentInfoType_t what, string param);

    //! hello message for the users.
    string getHelloMsg();

    //! execution information and version.
    string getAgentManagerInfo(agentInfoList_t *i);
        
  public:

    /*! \short   construct and initialize a Agent Manager object

        detailed constructor description for Agent server

        \arg \c argc - number of command line arguments
        \arg \c argv - list of command line arguments
    */
    Agent(int argc, char *argv[]);	

    /*! \short   destroy a Agent object
        detailed destructor description for agent
    */
    ~Agent();

    void run();

    //! dump a Agent Manager object
    void dump( ostream &os );
    
    //! handle the events
    void handleEvent(Event *e, fd_sets_t *fds);

};


//! overload for <<, so that a Agent object can be thrown into an ostream
std::ostream& operator<< ( std::ostream &os, Agent &obj );

};  // namespace auction

#endif // _AGENT_H_
