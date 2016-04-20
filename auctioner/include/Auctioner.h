
/*! \file   AuctionManager.h

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
    the main class 

    $Id: Auctioner.h 748 2015-07-23 14:33:00Z amarentes $
*/

#ifndef _Auctioner_H_
#define _Auctioner_H_


#include <iostream>
#include "Error.h"
#include "logfile.h"
#include "Logger.h"
#include "CommandLineArgs.h"
#include "ConfigManager.h"
#include "BiddingObjectManager.h"
#include "AuctionManager.h"
#include "SessionManager.h"
#include "CtrlComm.h"
#include "EventSchedulerAuctioner.h"
#include "Constants.h"
#include "AuctionManagerComponent.h"
#include "AUMProcessor.h"
#include "IpAp_template_container.h"
#include "AnslpClient.h"
#include "AnslpProcessor.h"


/*! \short   Auctioner class description
  
    detailed Auctioner class description
*/

namespace auction
{

typedef map<int, ipap_template_container*>   		  		auctionerTemplateList_t;
typedef map<int, ipap_template_container*>::iterator   		auctionerTemplateListIter_t;
typedef map<int, ipap_template_container*>::const_iterator   auctionerTemplateListConstIter_t;



class Auctioner
{
  public:

    // FIXME document!
    static int s_sigpipe[2];
 
  protected:
    
    //!< start time of the Auctioner
    time_t startTime;

    //! config file name
    string configFileName;

    //! log file name
    string logFileName;

    auto_ptr<Logger>          			log;
    auto_ptr<CommandLineArgs> 			args;
    auto_ptr<AuctionTimer>      		auct;
    auto_ptr<ConfigManager>   			conf;
    auto_ptr<BiddingObjectManager>     	bidm;
    auto_ptr<AuctionManager>   			aucm;
    auto_ptr<SessionManager>			sesm;
    auto_ptr<EventSchedulerAuctioner>  	evnt;
    auto_ptr<AnslpClient>				anslpc;
    
    auto_ptr<AUMProcessor> 				proc;    
    auto_ptr<CtrlComm>        			comm;
    auto_ptr<AnslpProcessor>        	anslproc;

    //! logging channel number used by objects of this class
    int ch;

	//! domain Id for exchanging ipap_messages
	int domainId;

     //! FD list (from AuctionManagerComponent.h)
    fdList_t fdList;

    //! 1 if the procedure for applying executing auctions runs in a separate thread
    int pprocThread;
    
    //! 1 if the procedure for applying receiving events from the anslp component runs in a separate thread
    int aprocThread;

    //! 1 if remote control interface is enabled
    static int enableCtrl;

	//! List of templates thta have been created for exchanging with other parties.
	auctionerTemplateList_t auctionerTemplates;

    //! signal handlers
    static void sigint_handler(int i);
    static void sigusr1_handler(int i);
    static void sigalarm_handler(int i);

    //! exit function called on exit
    static void exit_fct(void);

    //! return 1 if a Auction Manager is already running on the host
    int alreadyRunning();

    //! get info string for get_info response
    string getInfo(infoType_t what, string param);

    //! hello message for the users.
    string getHelloMsg();

    //! execution information and version.
    string getAuctionManagerInfo(infoList_t *i);
    
    //! activate/execute auctions
    void activateAuctions(auctionDB_t *auctions, EventScheduler *e);    


	void handleGetInfo(Event *e, fd_sets_t *fds);
    
	void handleGetModInfo(Event *e, fd_sets_t *fds);
    
	void handleAddBiddingObjects(Event *e, fd_sets_t *fds);      

    void handleAddAuctions(Event *e, fd_sets_t *fds);

	void handleAddBiddingObjectsCntrlComm(Event *e, fd_sets_t *fds);

	void handleAddBiddingObjectsAuction(Event *e, fd_sets_t *fds);

	void handleActivateBiddingObjects(Event *e, fd_sets_t *fds);
	
	void handleActivateAuction(Event *e, fd_sets_t *fds);

    void handleRemoveBiddingObjects(Event *e, fd_sets_t *fds);
    
    void handleRemoveBiddingObjectsAuction(Event *e, fd_sets_t *fds);

	void handleProcModeleTimer(Event *e, fd_sets_t *fds);

	void handlePushExecution(Event *e, fd_sets_t *fds);

	void handleSingleCheckSession(string sessionId, 
			anslp::mspec_rule_key key, anslp::anslp_ipap_message *ipap_mes, 
			anslp::objectList_t *objectList);

	void handleCreateCheckSession(Event *e, fd_sets_t *fds);
	
	void handleSingleCreateSession(string sessionId,
				anslp::mspec_rule_key key, anslp::anslp_ipap_message *ipap_mes, 
				anslp::objectList_t *objectList);
	
	void handleCreateSession(Event *e, fd_sets_t *fds);

	void handleRemoveSession(Event *e, fd_sets_t *fds);

	void handleSingleObjectAuctioningInteraction( string sessionId, anslp::anslp_ipap_message *ipap_mes);

	void handleAuctioningInteraction(Event *e, fd_sets_t *fds);
	
	void handleAddGeneratedBiddingObjects(Event *e, fd_sets_t *fds);
    
    void handleTransmitBiddingObjects(Event *e, fd_sets_t *fds);
		
  public:

    /*! \short   construct and initialize a Auction Manager object

        detailed constructor description for Auction server

        \arg \c argc - number of command line arguments
        \arg \c argv - list of command line arguments
    */
    Auctioner(int argc, char *argv[]);	

    /*! \short   destroy a Auctioner object
        detailed destructor description for auctioner
    */
    ~Auctioner();

    void run();

    //! dump a Auction Manager object
    void dump( ostream &os );
    
    //! handle the events
    void handleEvent(Event *e, fd_sets_t *fds);

};


//! overload for <<, so that a Auctioner object can be thrown into an ostream
std::ostream& operator<< ( std::ostream &os, Auctioner &obj );

}; // namespace auction

#endif // _Auctioner_H_
