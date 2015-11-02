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
#include "FieldDefParser.h"
#include "IpAp_message.h"

using namespace auction;

class AuctionFileParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( AuctionFileParser_Test );

	CPPUNIT_TEST( testParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void loadFieldDefs(fieldDefList_t *fieldList);

	void testParser();

  private:
    
    AuctionFileParser *ptrAuctionFileParser;
    ipap_template_container *templates;
    FieldDefParser *ptrFieldParsers;
    

    //! filter definitions
    fieldDefList_t fieldDefs;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( AuctionFileParser_Test );


void AuctionFileParser_Test::setUp() 
{
	const string filename = DEF_SYSCONFDIR "/example_auctions2.xml";

	try
	{
		int domain = 0;
		
		ptrAuctionFileParser = new AuctionFileParser(domain, filename);
						
		templates = new ipap_template_container();

		// load the filter def list
		loadFieldDefs(&fieldDefs);
		
		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void AuctionFileParser_Test::tearDown() 
{
	delete(ptrAuctionFileParser);
    delete(ptrFieldParsers);
	delete(templates);
	
}

void AuctionFileParser_Test::testParser() 
{
	auctionDB_t *new_auctions = new auctionDB_t();
		
	try
	{
		
		ptrAuctionFileParser->parse( &fieldDefs, new_auctions, templates );
		CPPUNIT_ASSERT( new_auctions->size() == 1 );
		
	}
	catch (Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void AuctionFileParser_Test::loadFieldDefs(fieldDefList_t *fieldList)
{
	const string filename = DEF_SYSCONFDIR "/fielddef.xml";

	try
	{
		ptrFieldParsers = new FieldDefParser(filename);
		ptrFieldParsers->parse(fieldList);
		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}

}
