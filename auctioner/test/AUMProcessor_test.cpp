/*
 * Test the AUMProcessor class.
 *
 * $Id: AUMProcessor_test.cpp 2015-08-17 11:44:00 amarentes $
 * $HeadURL: https://./test/AUMProcessor_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "AUMProcessor.h"
#include "Constants.h"
#include "ConstantsAum.h"
#include "BiddingObjectManager.h"
#include "ConfigManager.h"
#include "EventScheduler.h"
#include "AuctionManager.h"

using namespace auction;

class AUMProcessor_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( AUMProcessor_Test );

	CPPUNIT_TEST( test );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void test();

  private:

	  BiddingObject *ptrBid1;
      BiddingObject *ptrBid2;
      BiddingObject *ptrBid3;
      BiddingObject *ptrBid4;
      BiddingObject *ptrBid5;

	  ConfigManager *configManagerPtr;
	  AuctionManager *auctionManagerPtr;
	  BiddingObjectManager *bidManagerPtr;
	  AUMProcessor *aumProcessorPtr;
      auto_ptr<EventScheduler>  evnt;
	  
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( AUMProcessor_Test );


void AUMProcessor_Test::setUp() 
{
	
	try
	{
		string commandLine = "auctioner";
		char *cstr = new char[commandLine.length() + 1];
		strcpy(cstr, commandLine.c_str());
		time_t now = time(NULL);
		
		enum { kMaxArgs = 64 };
		int argc = 0;
		char *argv[kMaxArgs];

		char *p2 = strtok(cstr, " ");
		while (p2 && argc < kMaxArgs-1)
		{
			argv[argc++] = p2;
			p2 = strtok(0, " ");
		}
		argv[argc] = 0;
		
		int domain = 5;
		
		const string fieldname = DEF_SYSCONFDIR "/fielddef.xml";
		const string fieldValuename = DEF_SYSCONFDIR "/fieldval.xml";
				
		bidManagerPtr = new BiddingObjectManager(domain, fieldname,fieldValuename);

		const string filenameAgent1 = "../../etc/example_bids2.xml";
		biddingObjectDB_t *new_bidsAgent1 = bidManagerPtr->parseBiddingObjects(filenameAgent1);
				
		const string filenameAgent2 = "../../etc/example_bids3.xml";
		biddingObjectDB_t *new_bidsAgent2 = bidManagerPtr->parseBiddingObjects(filenameAgent2);

		const string filenameAgent3 = "../../etc/example_bids4.xml";
		biddingObjectDB_t *new_bidsAgent3 = bidManagerPtr->parseBiddingObjects(filenameAgent3);

		const string filenameAgent4 = "../../etc/example_bids5.xml";
		biddingObjectDB_t *new_bidsAgent4 = bidManagerPtr->parseBiddingObjects(filenameAgent4);

		const string filenameAgent5 = "../../etc/example_bids6.xml";
		biddingObjectDB_t *new_bidsAgent5 = bidManagerPtr->parseBiddingObjects(filenameAgent5);

		ptrBid1 = new BiddingObject(*((*new_bidsAgent1)[0]));
		ptrBid2 = new BiddingObject(*((*new_bidsAgent2)[0]));
		ptrBid3 = new BiddingObject(*((*new_bidsAgent3)[0]));
		ptrBid4 = new BiddingObject(*((*new_bidsAgent4)[0]));
		ptrBid5 = new BiddingObject(*((*new_bidsAgent5)[0]));
	
		const string configDTD = DEF_SYSCONFDIR "/netaum.conf.dtd";
		const string configFileName = NETAUM_DEFAULT_CONFIG_FILE;
		configManagerPtr = new ConfigManager(configDTD, configFileName, argv[0]);

		auto_ptr<EventScheduler> _evnt(new EventScheduler());
        evnt = _evnt;		
		
		ipap_template_container *templContainer = new ipap_template_container();
		
		int domainAuct = 8;
		
		auctionManagerPtr = new AuctionManager(domainAuct, fieldname, fieldValuename);

		const string filenameAuctions = "../../etc/example_auctions2.xml";
		
		auctionDB_t * auctions = auctionManagerPtr->parseAuctions(filenameAuctions, templContainer);
				
		Auction *auction = (*auctions)[0];
		
		auctionManagerPtr->addAuctions(auctions, evnt.get());
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 1 );
		
		aumProcessorPtr = new AUMProcessor(domainAuct,
										   configManagerPtr, 
										   configManagerPtr->getValue("FieldDefFile", "MAIN"),
										   configManagerPtr->getValue("FilterConstFile", "MAIN"),
										   configManagerPtr->isTrue("Thread","AUM_PROCESSOR"));
				
		aumProcessorPtr->addAuctionProcess( auction, evnt.get() );
		
        aumProcessorPtr->addBiddingObjectAuctionProcess(auction->getUId(), ptrBid1 );

        aumProcessorPtr->addBiddingObjectAuctionProcess(auction->getUId(), ptrBid2 );

        aumProcessorPtr->addBiddingObjectAuctionProcess(auction->getUId(), ptrBid3 );

        aumProcessorPtr->addBiddingObjectAuctionProcess(auction->getUId(), ptrBid4 );

        aumProcessorPtr->addBiddingObjectAuctionProcess(auction->getUId(), ptrBid5 );
		
		time_t stop = now + 200; 
									  
        aumProcessorPtr->executeAuction(auction->getUId(), now, stop, evnt.get());
        
	}
	catch(Error &e){
		cout << "Error:" << e.getError() << endl << flush;
	}

	
}

void AUMProcessor_Test::tearDown() 
{
	
	delete(bidManagerPtr);
	saveDelete(auctionManagerPtr);
	delete(aumProcessorPtr); //! This delete need the config Manager ptr, so 
							 //! first we need to delete this pointer.
	delete(configManagerPtr);
	evnt.reset();
		
}

void AUMProcessor_Test::test() 
{

	try
	{
		/*
		cout << "starting test" << endl;
		
		// Insert the auction.
		const string filename = DEF_SYSCONFDIR "/example_auctions1.xml";
		
		auctionDB_t * auctions = auctionManagerPtr->parseAuctions(filename);
				
		auctionManagerPtr->addAuctions(auctions, evnt.get());
				
		//CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 1 );
		
		// Insert Bids
		
		bidManagerPtr->addBid(ptrBid1);
		bidManagerPtr->addBid(ptrBid2);
		bidManagerPtr->addBid(ptrBid3);
		bidManagerPtr->addBid(ptrBid4);
		bidManagerPtr->addBid(ptrBid5);
		
		//CPPUNIT_ASSERT( bidManagerPtr->getNumBids() == 5 );
		
		cout << "ending test" << bidManagerPtr->getNumBids() << endl;
		*/
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
	
}

