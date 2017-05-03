
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
#include "BiddingObject.h"
#include "AuctionManagerComponent.h"
#include "Error.h"
#include "Logger.h"
#include "EventScheduler.h"
#include "IpApMessageParser.h"
#include "FieldDefManager.h"
#include "AuctionProcessObject.h"
#include "EventSchedulerAuctioner.h"

namespace auction
{

typedef enum
{
	AUM_SESSION_FIELD_SET_NAME = 0,
	AUM_REQUEST_FIELD_SET_NAME
} agentFieldSet_t;


class auctionProcess : public AuctionProcessObject
{
	public: 
		//! config params for module
		configParam_t *params;
		
		//! auction to start execution.
		Auction *auction; 
		
		//! Bids competing in the auction.
		auctioningObjectDB_t bids;  
		
		auctionProcess():AuctionProcessObject(), params(NULL), auction(NULL){ }
		
		~auctionProcess(){ }
		
		void setParams(configParam_t *_params){ params = _params; }
		
		configParam_t * getParams(){ return params; }
		
		void setAuction(Auction *_auction){ auction = _auction; }
		
		Auction * getAuction(){ return auction; }
		
		void insertBid(BiddingObject * bid){ bids.push_back(bid); }
		
		auctioningObjectDB_t * getBids() { return &bids; }
    
};

//! action list for each auction
typedef map<int, auctionProcess>            auctionProcessList_t;
typedef map<int, auctionProcess>::iterator  auctionProcessListIter_t;
typedef  map<int, auctionProcess>::reverse_iterator  auctionProcessListRevIter_t;

typedef map< agentFieldSet_t, set<ipap_field_key> >  		  setFieldsList_t;
typedef map< agentFieldSet_t, set<ipap_field_key> >::iterator  setFieldsListIter_t;



/*! \short   manage and execute algoirthms for a set of auctions.

    the AuctionProcessor class allows auctioning between bids 
*/

class AUMProcessor : public AuctionManagerComponent, public IpApMessageParser, public FieldDefManager
{
  private:

    //! We use as index for auction process the same auction id.
    
    //! associated module loader 
    //! this is the algorithm to execute for the bid
    ModuleLoader *loader;

    //! action of every auction being processed.
    auctionProcessList_t  auctions;
	
	miscList_t readMiscData( ipap_template *templ, ipap_data_record &record);
	
	bool intersects( time_t startDttmAuc, time_t stopDttmAuc, 
					 time_t startDttmReq, time_t stopDttmReq );
	
	bool forResource(string resourceAuc, string resourceIdReq);
	
  public:

	static setFieldsList_t fieldSets;

    /*! \short   construct and initialize a Auction Manager object

        \arg \c cnf        config manager
        \arg \c fdname     field definition file name
        \arg \c fvname     field value definition file name
        \arg \c threaded   run as separate thread
        \arg \c moduleDir  action module directory
    */
    AUMProcessor(int domain, ConfigManager *cnf, string fdname, 
					string fvname, int threaded, string moduleDir = "" );

    //!   destroy a Auction Processor object, to be overloaded
    virtual ~AUMProcessor();


    /*! \short   add a Auction to auction process list
        \arg \c a   pointer to auction
        \arg \c e   pointer to event scheduler (timer events)
        \returns 0 - on success, <0 - else
    */
    int addAuctionProcess( Auction *a, EventScheduler *e );


    //! execute the algorithm
    void executeAuction(int index, time_t start, time_t stop, EventScheduler *e );

    /*! \short   add a Bidding Object to auctio process biddding object list
        \arg \c index   index to add the element.
        \arg \c b 		Pointer to bidding object to insert
    */
    void addBiddingObjectAuctionProcess(int index, BiddingObject *b );

    /*! \short   delete a Bid to auction bid list
        \arg \c index   index to add the element.
        \arg \c  b 			   Pointer to bidding object to delete
    */
    void delBiddingObjectAuctionProcess( int index, BiddingObject *b );


    /*! \short   add a set of Bidding Objects to auction process list
        \arg \c index   index to add the element.
        \arg \c bids 	Pointer to bidding object list to insert
    */
    void addBiddingObjectsAuctionProcess(int index, auctioningObjectDB_t *bids );


    //! delete biddingObjects
    void delBiddingObjectsAuctionProcess( int index, auctioningObjectDB_t *bids );


    /*! \short   delete an Auction from the auction process list
        \arg \c index  index to erase.
    */
    void delAuctionProcess( int index, EventSchedulerAuctioner *e );
		
    /*! \short   get the auctions applicable given the options within the message.
        \arg \message a  pointer to a message with the options to filter.
        \returns 0 - list of application auctions.
    */
	auctioningObjectDB_t * getApplicableAuctions(ipap_message *message);


	//! gives an iterator over the first auction process registered.
	inline auctionProcessListIter_t begin(){ return auctions.begin(); }
	
	inline auctionProcessListIter_t end(){ return auctions.end(); }

    /*! \short   get the session information within the message.
        \arg \message a pointer to a message with the options to filter.
        \returns 0 - map of values that identify the session.
    */
	map<ipap_field_key,string> getSessionInformation(ipap_message *message);

    //! handle file descriptor event
    virtual int handleFDEvent(eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds);

    /*! \short delete the set of auctions from all request
     *  \arg \c aucts 		  auction set to delete.
     *  If a request process becomes free of auctions, then it is deleted too.
	 */
	void delAuctions(auctioningObjectDB_t *aucts, EventSchedulerAuctioner *e);


    //! thread main function
    void main();
   
    //! get information about load module
    string getInfo();

    //! dump a AUMProcessor object
    void dump( ostream &os );
	
    //! get the number of action modules currently in use
    int numModules() 
    { 
        return loader->numModules(); 
    }

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
