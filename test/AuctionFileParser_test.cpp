/*
 * Test the AuctionFileParser class.
 *
 * $Id: AuctionFileParser_test.cpp 2015-08-05 11:17:00 amarentes $
 * $HeadURL: https://./test/AuctionFileParser_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "AuctionFileParser.h"
#include "AuctionIdSource.h"


class AuctionFileParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( AuctionFileParser_Test );

	CPPUNIT_TEST( testParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testParser();

  private:
    
    AuctionFileParser *ptrAuctionFileParser;
    AuctionIdSource *idSource;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( AuctionFileParser_Test );


void AuctionFileParser_Test::setUp() 
{
	const string filename = DEF_SYSCONFDIR "/example_auctions1.xml";

	try
	{
		ptrAuctionFileParser = new AuctionFileParser(filename);
				
		idSource = new AuctionIdSource(1); // Unique.
		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void AuctionFileParser_Test::tearDown() 
{
	delete(ptrAuctionFileParser);
	delete(idSource);
	
}

void AuctionFileParser_Test::testParser() 
{
	auctionDB_t *new_auctions = new auctionDB_t();
		
	try
	{
		
		ptrAuctionFileParser->parse( new_auctions,idSource );
		
		CPPUNIT_ASSERT( new_auctions->size() == 1 );
		
	}
	catch (Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

