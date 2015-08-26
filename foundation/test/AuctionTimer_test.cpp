/*
 * Test the AuctionTimer class.
 *
 * $Id: AuctionTimer.cpp 2015-08-03 8:06:00 amarentes $
 * $HeadURL: https://./test/AuctionTimer_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "AuctionTimer.h"


class AuctionTimer_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( AuctionTimer_Test );

    CPPUNIT_TEST( testAuctionTimer );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testAuctionTimer();
	
  private:
        
    AuctionTimer *timer;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( AuctionTimer_Test );


void AuctionTimer_Test::setUp() 
{
	timer = AuctionTimer::getInstance();
	
}

void AuctionTimer_Test::tearDown() 
{
	delete(timer);
}

void AuctionTimer_Test::testAuctionTimer() 
{

	double clockSpeed = timer->getClockSpeed();
	
	int slots = 1;
		
	timer->start(slots);
	
	sleep(1);
	
	timer->stop(slots);
	
	unsigned long long latest = timer->latest(slots);
	
	unsigned long long avg = timer->avg(slots);
	
	CPPUNIT_ASSERT( latest == avg );

}





