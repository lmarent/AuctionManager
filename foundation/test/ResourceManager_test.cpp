/*
 * Test the AuctionManager class.
 *
 * $Id: AuctionManager.cpp 2015-08-03 11:48:00 amarentes $
 * $HeadURL: https://./test/ResourceManager_Test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "AuctionManager.h"


using namespace auction;

class ResourceManager_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( ResourceManager_Test );

    CPPUNIT_TEST( test );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void test();
	

  private:
    
	ResourceManager *resourceManagerPtr;
    auto_ptr<EventScheduler>  evnt;
	
        
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( ResourceManager_Test );


void ResourceManager_Test::setUp() 
{
	
	const string filename = DEF_SYSCONFDIR "/fielddef.xml";		
	const string fieldValuename = DEF_SYSCONFDIR "/fieldval.xml";
	
	
	try
	{
		int domain = 5;
		
		resourceManagerPtr = new ResourceManager(domain, filename, fieldValuename);
		auto_ptr<EventScheduler> _evnt(new EventScheduler());
        evnt = _evnt;
				
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void ResourceManager_Test::tearDown() 
{

	saveDelete(resourceManagerPtr);
	
	evnt.reset();

}

void ResourceManager_Test::test() 
{

	try
	{
		Resource *resource1 = new Resource("set1","res1");
		Resource *resource2 = new Resource("set1","res2");
		
		auctioningObjectDB_t * auctions = new auctioningObjectDB_t();
		auctions->push_back(resource1);
		auctions->push_back(resource2);
				
		CPPUNIT_ASSERT( auctions->size() == 2 );
				
		resourceManagerPtr->addAuctioningObjects(auctions, evnt.get());				
		CPPUNIT_ASSERT( resourceManagerPtr->getNumAuctioningObjects() == 2 );
						
		// Delete functions
				
		resourceManagerPtr->delResource(0,evnt.get());
		
		CPPUNIT_ASSERT( resourceManagerPtr->getNumAuctioningObjects() == 1 );
		
		resourceManagerPtr->delResource("set1", "res2", evnt.get());
	
		CPPUNIT_ASSERT( resourceManagerPtr->getNumAuctioningObjects() == 0 );
		
		// Reinsert resources - Delete by set name

		auctioningObjectDB_t * auctions2 = new auctioningObjectDB_t();		
		
		Resource *resource3 = new Resource("set1","res3");
		
		auctions2->push_back(resource3);
		
		CPPUNIT_ASSERT( auctions2->size() == 1 );
						
		resourceManagerPtr->addAuctioningObjects(auctions2, evnt.get());
				
		CPPUNIT_ASSERT( resourceManagerPtr->getNumAuctioningObjects() == 1 );
				
		auctioningObjectDB_t auctions3 = resourceManagerPtr->getAuctioningObjects();

		Resource *resourcePtr = dynamic_cast<Resource *>(auctions3[0]);
		
		Resource *resource4 = new Resource(*resourcePtr);
		
		resource4->setSet("set2");
		
		resourceManagerPtr->addAuctioningObject(resource4);
		
		CPPUNIT_ASSERT( resourceManagerPtr->getNumAuctioningObjects() == 2 );
				
		resourceManagerPtr->delResources(resource4->getSet(), evnt.get());
								
		CPPUNIT_ASSERT( resourceManagerPtr->getNumAuctioningObjects() == 1 );
				
		// resource - Delete by pointer
		
		auctions3 = resourceManagerPtr->getAuctioningObjects();
		
		resourcePtr = dynamic_cast<Resource *>(auctions3[0]);
						
		resourceManagerPtr->delResource(resourcePtr, evnt.get());
		
		CPPUNIT_ASSERT( resourceManagerPtr->getNumAuctioningObjects() == 0 );
				
		// Verifies intervals for auction within the same resource

		int domain = 5;
		string filename1 = DEF_SYSCONFDIR "/fielddef.xml";		
		string fieldValuename1 = DEF_SYSCONFDIR "/fieldval.xml";
	
		AuctionManager *auctionManagerPtr;
		auctionManagerPtr = new AuctionManager(domain, filename1, fieldValuename1);

		ipap_template_container *templates;
		
		const string filename = "../../etc/example_auctions6.xml";
		templates = new ipap_template_container();
		auctioningObjectDB_t * auctions4 = auctionManagerPtr->parseAuctions(filename, templates);
		CPPUNIT_ASSERT( auctions4->size() == 2 );
		saveDelete(templates);

		Resource *resource5 = new Resource("set1","res5");
		
		Auction * auctionPtr = dynamic_cast<Auction *>((*auctions4)[0]);
		CPPUNIT_ASSERT( resource5->verifyAuction(auctionPtr) == true );
		resource5->addAuction(auctionPtr);
		
		auctionPtr = dynamic_cast<Auction *>((*auctions4)[1]);
		CPPUNIT_ASSERT( resource5->verifyAuction(auctionPtr) == false );
		saveDelete(auctionManagerPtr);	 
		
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}

}
