
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


#include "AuctionManagerComponent.h"
#include "IdSource.h"
#include "EventScheduler.h"
#include "ProcModule.h"
#include "ModuleLoader.h"
#include "FieldDefManager.h"
#include "BiddingObject.h"
#include "AuctionProcessObject.h"

namespace auction
{

class requestProcess : public AuctionProcessObject
{
	public: 
		//! for reference we set the id of the session
		string sessionId;
		
		//! Name of the module;
		string moduleName;			 		
		
		//! start time;
		time_t start;
		
		//! end time;
		time_t stop;
		
		//! Parameters for execution
		fieldList_t *parameters; 	 		
		
		//! Auctions to execute 
		auctionDB_t auctions;  		 	
    
		requestProcess(fieldList_t * _parameters=NULL): 
			AuctionProcessObject(), moduleName(), parameters(_parameters){}
		
		~requestProcess(){}
		
		void setStart(time_t start_t){ start = start_t; }
		
		time_t getStart(void){ return start; }
		
		void setStop(time_t stop_t){ stop = stop_t; }
		
		time_t getStop(void){ return stop; }
		
		string getSession(){ return sessionId; }
		
		void setSession(string _sessionId){ sessionId = _sessionId; }
		
		string getModuleName(){ return moduleName; }
		
		void setModuleName(string _moduleName){ moduleName = _moduleName; }
		
		fieldList_t * getParameters() { return parameters; }
		
		void setParameters(fieldList_t *_parameters){ parameters = _parameters; }
		
		auctionDB_t * getAuctions(){ return &auctions; }
		
		void insertAuction(Auction *auction){ auctions.push_back(auction); }
		
};

//! action list for each auction
typedef map<int, requestProcess>            requestProcessList_t;
typedef map<int, requestProcess>::iterator  requestProcessListIter_t;



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
    requestProcessList_t requests;

    //! pool of unique request process 
    IdSource idSource;
    
    //! identifies uniquely biddings from this agent.
    int domain;

  public:

    /*! \short   construct and initialize a AgentProcessor object

        \arg \c cnf        config manager
        \arg \c fdname     field definition file name
        \arg \c fvname     field value definition file name
        \arg \c threaded   run as separate thread
    */
    AgentProcessor(int domain, ConfigManager *cnf, string fdname, string fvname, int threaded, string moduleDir = "" );

    //!   destroy a Agent Processor object, to be overloaded
    virtual ~AgentProcessor();

    //! Add a new request to the list of request to execute.
    int addRequest( string sessionId, auction::fieldList_t *parameters, Auction *auction, time_t start, time_t stop );

    //! delete request
    void delRequest( int index );

    //! release the module attached to the request
    void releaseRequest( int index );

    //! execute the algorithm
    void executeRequest( int index, EventScheduler *e );

    /*! \short  add a Auction to auction  list
     *  \arg \c index 		  Id for the request.
        \arg \c a 			  Pointer to auction to insert
    */
    void addAuctionRequest(int index, Auction *a );

    /*! \short  add auctions to a request
     *  \arg \c index 		  Id for the request.
        \arg \c auctions 	  List of auctions to add. 
    */
	void addAuctionsRequest( int index,  auctionDB_t *auctions );

    /*! \short   	delete an Auction from the request process list
		\arg index  Id for the request.
        \arg \c 	a  pointer to auction
    */
    void delAuctionRequest( int index,  Auction *a );

    /*! \short delete auctions from a request
     *  \arg \c index 		  Id for the request.
	 */
    void delAuctionsRequest( int index,  auctionDB_t *auctions );	
	
    /*! \short delete the set of auctions from all request
     *  \arg \c aucts 		  auction set to delete.
     *  If a request process becomes free of auctions, then it is deleted too.
	 */
	void delAuctions(auctionDB_t *aucts);

	//! get the sessionId generating a request process
	string getSession(int index);

    //! handle file descriptor event
    virtual int handleFDEvent(eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds);

	//! gives an iterator over the first process request registered.
	inline requestProcessListIter_t begin(){ return requests.begin(); }
	
	inline requestProcessListIter_t end(){ return requests.end(); }
	
	inline int getNumRequestProcess(void) { return requests.size(); }
	
    //! thread main function
    void main();
   
    //! Get the domain as a string
    string getDomainStr();
   
    //! get information about load module
    string getInfo();

    //! dump a AUMProcessor object
    void dump( ostream &os );

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
