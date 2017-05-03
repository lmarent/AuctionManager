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
		int domain = 5;
		
		auctionManagerPtr = new AuctionManager(domain, filename, fieldValuename);
		auto_ptr<EventScheduler> _evnt(new EventScheduler());
        evnt = _evnt;
				
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
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
		
		const string filename = "../../etc/example_auctions1.xml";
		templates = new ipap_template_container();
		auctioningObjectDB_t * auctions = auctionManagerPtr->parseAuctions(filename, templates);
		CPPUNIT_ASSERT( auctions->size() == 1 );
		saveDelete(templates);
				
		auctionManagerPtr->addAuctioningObjects(auctions, evnt.get());				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 1 );
		
		auctioningObjectDB_t auctions2 = auctionManagerPtr->getAuctioningObjects();		

		Auction *auctionPtr = dynamic_cast<Auction *>(auctions2[0]);
						
		Auction *auction2 = new Auction(*auctionPtr);
		
		auction2->setName("2");
		
		// test the increment and decrement functions for auctions.
		
		auction2->incrementSessionReferences("session1");
		auction2->incrementSessionReferences("session2");
		auction2->incrementSessionReferences("session3");
		
		CPPUNIT_ASSERT( auction2->getSessionReferences() == 3 );
		
		auction2->decrementSessionReferences("session2");
		
		CPPUNIT_ASSERT( auction2->getSessionReferences() == 2 );
						
		auctionManagerPtr->addAuctioningObject(auction2);
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 2 );
				
		string info = auctionPtr->getInfo();

		cout << "Info 1:" << info << endl;
		cout << "id:" << auctionPtr->getUId() << endl;
		cout << "Info 2:" << auctionManagerPtr->getInfo(auctionPtr->getUId()) << endl;
						
		CPPUNIT_ASSERT( info.compare(auctionManagerPtr->getInfo(auctionPtr->getUId())) == 0 );
		
		auctioningObjectDB_t auctions3 = auctionManagerPtr->getAuctioningObjects();	
		
		CPPUNIT_ASSERT( auctions3.size() == 2 );
				
		// Delete functions
				
		auctionManagerPtr->delAuction(auctionPtr->getUId(),evnt.get());
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 1 );
		
		auctionManagerPtr->delAuction(auction2->getSet(), 
										auction2->getName(), evnt.get());
	
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 0 );
		
		// Reinsert auctions - Delete by set name( Bidder )

		templates = new ipap_template_container();

		auctions = auctionManagerPtr->parseAuctions(filename, templates);
		
		saveDelete(templates);
				
		auctionManagerPtr->addAuctioningObjects(auctions, evnt.get());
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 1 );
		
		auctions2 = auctionManagerPtr->getAuctioningObjects();		

		auctionPtr = dynamic_cast<Auction *>(auctions2[0]);
		
		auction2 = new Auction(*auctionPtr);
		
		auction2->setName("2");
		
		auctionManagerPtr->addAuctioningObject(auction2);
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 2 );
				
		auctionManagerPtr->delAuctions(auction2->getSet(), evnt.get());
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 0 );
				
		// Reinsert auction - Delete by pointer
		
		templates = new ipap_template_container();

		auctions = auctionManagerPtr->parseAuctions(filename, templates);
		
		saveDelete(templates);
		
		auctionManagerPtr->addAuctioningObjects(auctions, evnt.get());
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 1 );
		
		auctions2 = auctionManagerPtr->getAuctioningObjects();		

		auctionPtr = dynamic_cast<Auction *>(auctions2[0]);
		
		auctionManagerPtr->delAuction(auctionPtr, evnt.get());
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 0 );
		
		// Reinsert auctions - Delete by set.
		
		templates = new ipap_template_container();
		
		auctions = auctionManagerPtr->parseAuctions(filename, templates);
		
		saveDelete(templates);
		
		auctionManagerPtr->addAuctioningObjects(auctions, evnt.get());
				
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 1 );
				
		auctionPtr = dynamic_cast<Auction *>(
						auctionManagerPtr->getAuctioningObject("general1","1"));
				
		if (auctionPtr != NULL){
			auction2 = new Auction(*auctionPtr);
			auction2->setName("2");	
			auctionManagerPtr->addAuctioningObject(auction2);		
			CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 2 );
		}
						
		auctions3 = auctionManagerPtr->getAuctioningObjects();	
		
		auctionManagerPtr->delAuctioningObjects(&auctions3, evnt.get());
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 0 );
		
		
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}


}





