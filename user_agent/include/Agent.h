
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
#include "BiddingObjectManager.h"
#include "AuctionManager.h"
#include "MAPIAuctionParser.h"
#include "CtrlComm.h"
#include "AuctionManagerComponent.h"
#include "AgentProcessor.h"
#include "AnslpClient.h"
#include "ResourceRequestManager.h"
#include "SessionManager.h"
#include "MAPIResourceRequestParser.h"
#include "EventSchedulerAgent.h"
#include "AgentSessionManager.h"




namespace auction
{

typedef map<int, ipap_template_container* >   		  			agentTemplateList_t;
typedef map<int, ipap_template_container* >::iterator   		agentTemplateListIter_t;
typedef map<int, ipap_template_container* >::const_iterator    	agentTemplateListConstIter_t;


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

    auto_ptr<Logger>          						log;
    auto_ptr<CommandLineArgs> 						args;
    auto_ptr<AuctionTimer>      					auct;
	auto_ptr<ConfigManager>   						conf;
    auto_ptr<ResourceRequestManager>   				rreqm;
    auto_ptr<AuctionManager>   						aucm;
    auto_ptr<BiddingObjectManager>     				bidm;
    auto_ptr<AgentSessionManager>   				asmp;
    auto_ptr<EventSchedulerAgent>  					evnt;
    auto_ptr<AnslpClient>							anslpc;
    
    auto_ptr<AgentProcessor> 						proc;    
    auto_ptr<CtrlComm>        						comm;
    

    
    //! logging channel number used by objects of this class
    int ch;

	//! domain Id for exchanging ipap_messages
	int domainId;

     //! FD list (from AuctionManagerComponent.h)
    fdList_t fdList;

    //! 1 if the procedure for applying executing auctions runs in a separate thread
    int pprocThread;

    //! 1 if remote control interface is enabled
    static int enableCtrl;

    //! List of templates exchanged with different domains.
    agentTemplateList_t agentTemplates;

	// defaults values from the configuration file.
	string defaultSourceAddr;
	string defaultDestinAddr;
	uint16_t defaultDestinPort;
	uint16_t defaultSourcePort;
	uint8_t defaultProtocol;
	uint32_t defaultLifeTime;

    // signal handlers
    static void sigint_handler(int i);
    static void sigusr1_handler(int i);
    static void sigalarm_handler(int i);

    // exit function called on exit
    static void exit_fct(void);

	//! Read default data from configuration file.
	void readDefaultData(void);

    //! return 1 if a Agent Manager is already running on the host
    int alreadyRunning();

    //! get info string for get_info response
    string getInfo(agentInfoType_t what, string param);

    //! hello message for the users.
    string getHelloMsg();

    //! execution information and version.
    string getAgentManagerInfo(agentInfoList_t *i);
    
    void handleGetInfo(Event *e, fd_sets_t *fds);
    
            
    //! handle the addition of resource requests.
    void handleAddResourceRequests(Event *e, fd_sets_t *fds);

    //! handle the activation of resource request intervals.
    void handleActivateResourceRequestInterval(Event *e);

	//! handle the response for a session creation previously sent.
	void handleResponseCreateSession(Event *e, fd_sets_t *fds);

    //! handle the addition of auctions.
    void handleAddAuctions(Event *e, fd_sets_t *fds);

    //! handle the activation of auctions.
    void handleActivateAuctions(Event *e);

    //! Execute the bid process.
    void handlePushExecution(Event *e, fd_sets_t *fds);
	
	//! Remove push execution event.
	void handleRemovePushExecution(Event *e, fd_sets_t *fds);
	
    //! handle the addition of generated bidding objects
    void handleAddGeneratedBiddingObjects(Event *e, fd_sets_t *fds);

    //! handle the activation of bidding objects
    void handleActivateBiddingObjects(Event *e, fd_sets_t *fds);
    
    //! handle the transmission of generated bidding objects
    void handleTransmitBiddingObjects(Event *e, fd_sets_t *fds);
    
    //! handle the remove auctions.
    void handleRemoveAuctions(Event *e);

    //! handle the addition of bidding objects.
    void handleAddBiddingObjects(Event *e, fd_sets_t *fds);

    //! handle the delete of biddingObjects
    void handleRemoveBiddingObjects(Event *e);

    //! handle the remove of resource request intervals.
    void handleRemoveResourceRequestInterval(Event *e);
    
    //! handle the interaction arrived from a auctioneer system.
    void handleAuctioningInteraction(Event *e, fd_sets_t *fds);
        
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

}  // namespace auction

#endif // _AGENT_H_
