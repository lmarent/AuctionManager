/*
 * Test the BidFileParser class.
 *
 * $Id: BidFileParser_test.cpp 2015-07-24 15:50:00 amarentes $
 * $HeadURL: https://./test/BidFileParser_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "BidFileParser.h"


class BidFileParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( BidFileParser_Test );

	CPPUNIT_TEST( testParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testParser();

  private:
    
    BidFileParser *ptrBidFileParser;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( BidFileParser_Test );


void BidFileParser_Test::setUp() 
{
	const string filename = DEF_SYSCONFDIR "/example_bids1.xml";

	try
	{
		
		ptrBidFileParser = new BidFileParser(filename);
		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void BidFileParser_Test::tearDown() 
{
	delete(ptrBidFileParser);
}

void BidFileParser_Test::testParser() 
{

}


