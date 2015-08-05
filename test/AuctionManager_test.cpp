/*
 * Test the AuctionManager class.
 *
 * $Id: AuctionManager.cpp 2015-08-03 11:48:00 amarentes $
 * $HeadURL: https://./test/AuctionManager_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "AuctionManager.h"


class AuctionManager_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( AuctionManager_Test );

    CPPUNIT_TEST( test );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void test();
	

  private:
    
	AuctionManager *auctionManagerPtr;
    auto_ptr<EventScheduler>  evnt;
	
        
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( AuctionManager_Test );


void AuctionManager_Test::setUp() 
{
	
	const string filename = DEF_SYSCONFDIR "/example_bids1.xml";
		
	try
	{
		auctionManagerPtr = new AuctionManager();
		auto_ptr<EventScheduler> _evnt(new EventScheduler());
        evnt = _evnt;
				
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void AuctionManager_Test::tearDown() 
{
	delete(auctionManagerPtr);
	
}

void AuctionManager_Test::test() 
{

	try
	{
		const string filename = DEF_SYSCONFDIR "/example_auctions1.xml";
		
		auctionDB_t * auctions = auctionManagerPtr->parseAuctions(filename);
		
		cout << "Auctions parsed " << endl;
		
		auctionManagerPtr->addAuctions(auctions, evnt.get());

		cout << "Auctions Added " << endl;
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 1 );
		
		//string info = auctionPtr->getInfo();
		
		// cout << "Info: " << info << endl;
	
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}


}





