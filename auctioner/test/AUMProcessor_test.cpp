/*
 * Test the AUMProcessor class.
 *
 * $Id: AUMProcessor_test.cpp 2015-08-17 11:44:00 amarentes $
 * $HeadURL: https://./test/AUMProcessor_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "AUMProcessor.h"
#include "Constants.h"
#include "BidManager.h"
#include "ConfigManager.h"
#include "EventScheduler.h"
#include "AuctionManager.h"


class AUMProcessor_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( AUMProcessor_Test );

	CPPUNIT_TEST( test );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void test();

  private:

	  Bid *ptrBid1;
      Bid *ptrBid2;
      Bid *ptrBid3;
      Bid *ptrBid4;
      Bid *ptrBid5;

	  ConfigManager *configManagerPtr;
	  AuctionManager *auctionManagerPtr;
	  BidManager *bidManagerPtr;
	  AUMProcessor *aumProcessorPtr;
      auto_ptr<EventScheduler>  evnt;
	  
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( AUMProcessor_Test );


void AUMProcessor_Test::setUp() 
{
	
	try
	{
		cout << "Start setup" << endl;
		string commandLine = "auctioner";
		char *cstr = new char[commandLine.length() + 1];
		strcpy(cstr, commandLine.c_str());
		
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
		
		const string fieldname = DEF_SYSCONFDIR "/fielddef.xml";
		const string fieldValuename = DEF_SYSCONFDIR "/fieldval.xml";
				
		bidManagerPtr = new BidManager(fieldname,fieldValuename);

		const string filenameAgent1 = DEF_SYSCONFDIR "/example_bids2.xml";
		bidDB_t *new_bidsAgent1 = bidManagerPtr->parseBids(filenameAgent1);
				
		const string filenameAgent2 = DEF_SYSCONFDIR "/example_bids3.xml";
		bidDB_t *new_bidsAgent2 = bidManagerPtr->parseBids(filenameAgent2);

		const string filenameAgent3 = DEF_SYSCONFDIR "/example_bids4.xml";
		bidDB_t *new_bidsAgent3 = bidManagerPtr->parseBids(filenameAgent3);

		const string filenameAgent4 = DEF_SYSCONFDIR "/example_bids5.xml";
		bidDB_t *new_bidsAgent4 = bidManagerPtr->parseBids(filenameAgent4);

		const string filenameAgent5 = DEF_SYSCONFDIR "/example_bids6.xml";
		bidDB_t *new_bidsAgent5 = bidManagerPtr->parseBids(filenameAgent5);

		ptrBid1 = new Bid(*((*new_bidsAgent1)[0]));
		ptrBid2 = new Bid(*((*new_bidsAgent2)[0]));
		ptrBid3 = new Bid(*((*new_bidsAgent3)[0]));
		ptrBid4 = new Bid(*((*new_bidsAgent4)[0]));
		ptrBid5 = new Bid(*((*new_bidsAgent5)[0]));
	
		const string configFileName = NETAUM_DEFAULT_CONFIG_FILE;
		configManagerPtr = new ConfigManager(configFileName, argv[0]);

		auto_ptr<EventScheduler> _evnt(new EventScheduler());
        evnt = _evnt;		
		
		auctionManagerPtr = new AuctionManager();

		const string filenameAuctions = DEF_SYSCONFDIR "/example_auctions2.xml";
		
		auctionDB_t * auctions = auctionManagerPtr->parseAuctions(filenameAuctions);
				
		Auction *auction = (*auctions)[0];
		
		auctionManagerPtr->addAuctions(auctions, evnt.get());
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 1 );
		
		aumProcessorPtr = new AUMProcessor(configManagerPtr, 
											configManagerPtr->getValue("FieldDefFile", "MAIN"),
											configManagerPtr->isTrue("Thread","AUM_PROCESSOR"));
		
		cout << "Auction Id:" << auction->getUId() << endl;
		
		aumProcessorPtr->addAuctions( auctions, evnt.get() );
		
        aumProcessorPtr->addBidAuction(auction->getSetName(), 
										auction->getAuctionName(), 
										  ptrBid1 );

        aumProcessorPtr->addBidAuction(auction->getSetName(), 
										auction->getAuctionName(), 
										  ptrBid2 );

        aumProcessorPtr->addBidAuction(auction->getSetName(), 
										auction->getAuctionName(), 
										  ptrBid3 );

        aumProcessorPtr->addBidAuction(auction->getSetName(), 
										auction->getAuctionName(), 
										  ptrBid4 );

        aumProcessorPtr->addBidAuction(auction->getSetName(), 
										auction->getAuctionName(), 
										  ptrBid5 );
									  
        aumProcessorPtr->executeAuction(auction->getUId(), 
										 auction->getAuctionName());
        
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

