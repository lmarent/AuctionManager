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
	CPPUNIT_TEST( testMultipleAuctions );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void loadFieldDefs(fieldDefList_t *fieldList);

	void testParser();
	void testMultipleAuctions();

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
	auctioningObjectDB_t *new_auctions = new auctioningObjectDB_t();
		
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

void AuctionFileParser_Test::testMultipleAuctions() 
{
	auctioningObjectDB_t new_auctions;
		
	try
	{

		int domain = 0;
		
		string filename2 = DEF_SYSCONFDIR "/example_auctions4.xml";
		
		AuctionFileParser *ptrAuctionFileParser2 = new AuctionFileParser(domain, filename2);
						
		ipap_template_container *templates2 = new ipap_template_container();

		ptrAuctionFileParser2->parse( &fieldDefs, &new_auctions, templates );
		
		// Verify the number of auctions loaded
		CPPUNIT_ASSERT( new_auctions.size() == 3 );
		
		// Verify the interval for the first auction. start and end time
		Auction *first = dynamic_cast<Auction*>(new_auctions[0]);
		Auction *second = dynamic_cast<Auction*>(new_auctions[1]);
		Auction *third = dynamic_cast<Auction*>(new_auctions[2]);
		
		time_t start1 = first->getStart();
		time_t stop1 = first->getStop();
		string aucName = first->getName();

		time_t now = time(NULL);
				
		CPPUNIT_ASSERT( (start1 >= now + 9) && (start1 <= now + 20) );
		CPPUNIT_ASSERT( stop1 == start1 + 10 );
		CPPUNIT_ASSERT( aucName.compare("10") == 0 );
		
		start1 = second->getStart();
		stop1 = second->getStop();
		aucName = second->getName();

		CPPUNIT_ASSERT( (start1 >= now + 19) && (start1 <= now + 30) );
		CPPUNIT_ASSERT( start1 + 10 );
		CPPUNIT_ASSERT( aucName.compare("11") == 0 );

		start1 = third->getStart();
		stop1 = third->getStop();
		aucName = third->getName();
				
		CPPUNIT_ASSERT( start1 > now - 10 );
		CPPUNIT_ASSERT( stop1 == start1 + 10 );
		CPPUNIT_ASSERT( aucName == "12" );

		
		// Verify the interval for the second auction. start and end time
		
		delete(ptrAuctionFileParser2);
		delete(templates2);
		
	}
	catch (Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}
