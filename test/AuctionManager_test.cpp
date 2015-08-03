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

    CPPUNIT_TEST( AuctionManager_Test );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void AuctionManager_Test();
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);
	

  private:
    
	AuctionManager *auctionManagerPtr;
        
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( AuctionManager_Test );


void AuctionManager_Test::setUp() 
{
		
	try
	{
		auctionManagerPtr = new AuctionManager(0, "");
		
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void AuctionManager_Test::tearDown() 
{
	delete(auctionManagerPtr);
	
}

void AuctionManager_Test::testBidManager() 
{

	try
	{
	
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}


}





