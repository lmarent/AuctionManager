/*
 * Test the AuctionManager class.
 *
 * $Id: AuctionManager.cpp 2015-08-03 11:48:00 amarentes $
 * $HeadURL: https://./test/AuctionManager_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "AuctionManager.h"

using namespace auction;

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
	
	const string filename = DEF_SYSCONFDIR "/fielddef.xml";		
	const string fieldValuename = DEF_SYSCONFDIR "/fieldval.xml";
	
	
	try
	{
		auctionManagerPtr = new AuctionManager(filename, fieldValuename);
		auto_ptr<EventScheduler> _evnt(new EventScheduler());
        evnt = _evnt;
				
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void AuctionManager_Test::tearDown() 
{

	saveDelete(auctionManagerPtr);
	
	evnt.reset();

}

void AuctionManager_Test::test() 
{

	try
	{
		ipap_template_container *templates;
		
		const string filename = DEF_SYSCONFDIR "/example_auctions1.xml";
		
		templates = new ipap_template_container();
		
		auctionDB_t * auctions = auctionManagerPtr->parseAuctions(filename, templates);
		
		cout << "Num templates: " << templates->get_num_templates() << endl;
		
		saveDelete(templates);
				
		auctionManagerPtr->addAuctions(auctions, evnt.get());
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 1 );
		
		auctionDB_t auctions2 = auctionManagerPtr->getAuctions();		

		Auction *auctionPtr = auctions2[0];
		
		Auction *auction2 = new Auction(*auctionPtr);
		
		auction2->setAuctionName("2");
		
		// test the increment and decrement functions for auctions.
		
		auction2->incrementSessionReferences("session1");
		auction2->incrementSessionReferences("session2");
		auction2->incrementSessionReferences("session3");
		
		CPPUNIT_ASSERT( auction2->getSessionReferences() == 3 );
		
		auction2->decrementSessionReferences("session2");
		
		CPPUNIT_ASSERT( auction2->getSessionReferences() == 2 );
						
		auctionManagerPtr->addAuction(auction2);
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 2 );
				
		string info = auctionPtr->getInfo();
						
		CPPUNIT_ASSERT( info.compare(auctionManagerPtr->getInfo(auctionPtr->getUId())) == 0 );
		
		auctionDB_t auctions3 = auctionManagerPtr->getAuctions();	
		
		CPPUNIT_ASSERT( auctions3.size() == 2 );
				
		// Delete functions
				
		auctionManagerPtr->delAuction(auctionPtr->getUId(),evnt.get());
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 1 );
		
		auctionManagerPtr->delAuction(auction2->getSetName(), 
										auction2->getAuctionName(), evnt.get());
	
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 0 );
		
		// Reinsert auctions - Delete by set name( Bidder )

		templates = new ipap_template_container();

		auctions = auctionManagerPtr->parseAuctions(filename, templates);
		
		saveDelete(templates);
				
		auctionManagerPtr->addAuctions(auctions, evnt.get());
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 1 );
		
		auctions2 = auctionManagerPtr->getAuctions();		

		auctionPtr = auctions2[0];
		
		auction2 = new Auction(*auctionPtr);
		
		auction2->setAuctionName("2");
		
		auctionManagerPtr->addAuction(auction2);
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 2 );
				
		auctionManagerPtr->delAuctions(auction2->getSetName(), evnt.get());
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 0 );
				
		// Reinsert auction - Delete by pointer
		
		templates = new ipap_template_container();

		auctions = auctionManagerPtr->parseAuctions(filename, templates);
		
		saveDelete(templates);
		
		auctionManagerPtr->addAuctions(auctions, evnt.get());
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 1 );
		
		auctions2 = auctionManagerPtr->getAuctions();		

		auctionPtr = auctions2[0];
		
		auctionManagerPtr->delAuction(auctionPtr, evnt.get());
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 0 );
		
		// Reinsert auctions - Delete by set.
		
		templates = new ipap_template_container();
		
		auctions = auctionManagerPtr->parseAuctions(filename, templates);
		
		saveDelete(templates);
				
		auctionManagerPtr->addAuctions(auctions, evnt.get());
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 1 );
				
		auctionPtr = auctionManagerPtr->getAuction("general","1");
				
		auction2 = new Auction(*auctionPtr);
		
		auction2->setAuctionName("2");
				
		auctionManagerPtr->addAuction(auction2);
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 2 );
						
		auctions3 = auctionManagerPtr->getAuctions();	
		
		auctionManagerPtr->delAuctions(&auctions3, evnt.get());
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctions() == 0 );
		
		
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}


}





