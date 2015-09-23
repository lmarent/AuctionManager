/*
 * Test the AuctionIdSource class.
 *
 * $Id: AuctionIdSource_test.cpp 2015-08-04 14:56:00 amarentes $
 * $HeadURL: https://./test/AuctionIdSource_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "AuctionIdSource.h"


using namespace auction;

class AuctionIdSource_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( AuctionIdSource_Test );

	CPPUNIT_TEST( testId );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void testId();

  private:
    
    AuctionIdSource *ptrAuctionIdSourceNonUnique;
    AuctionIdSource *ptrAuctionIdSourceUnique;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( AuctionIdSource_Test );


void AuctionIdSource_Test::setUp() 
{

	// 0 defines that the object should not create unique Id
	ptrAuctionIdSourceNonUnique = new AuctionIdSource(0);
	
	// 1 defines that the object should create unique Id
	ptrAuctionIdSourceUnique = new AuctionIdSource(1);
	

}

void AuctionIdSource_Test::tearDown() 
{
	delete(ptrAuctionIdSourceNonUnique);
	delete(ptrAuctionIdSourceUnique);
		
}

void AuctionIdSource_Test::testId() 
{
	// Non Unique test
	unsigned short id1 = ptrAuctionIdSourceNonUnique->newId();
	unsigned short id2 = ptrAuctionIdSourceNonUnique->newId();
	unsigned short id3 = ptrAuctionIdSourceNonUnique->newId();	
	ptrAuctionIdSourceNonUnique->freeId(id1);
	unsigned short id4 = ptrAuctionIdSourceNonUnique->newId();
	CPPUNIT_ASSERT( id1 == id4 );
	
	
	// Unique test
	unsigned short id5 = ptrAuctionIdSourceUnique->newId();
	unsigned short id6 = ptrAuctionIdSourceUnique->newId();
	unsigned short id7 = ptrAuctionIdSourceUnique->newId();	
	ptrAuctionIdSourceUnique->freeId(id5);
	unsigned short id8 = ptrAuctionIdSourceUnique->newId();
	CPPUNIT_ASSERT( id5 != id8 );
	
	
}

