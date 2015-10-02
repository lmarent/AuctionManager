/*
 * Test the BidFileParser class.
 *
 * $Id: BidFileParser_test.cpp 2015-07-24 15:50:00 amarentes $
 * $HeadURL: https://./test/BidFileParser_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "BidManager.h"
#include "BidFileParser.h"
#include "FieldValParser.h"
#include "FieldDefParser.h"
#include "BidIdSource.h"

using namespace auction;

class BidFileParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( BidFileParser_Test );

	CPPUNIT_TEST( testParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);

	void testParser();

  private:
    
    BidFileParser *ptrBidFileParser;
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;
    BidIdSource *idSource;

    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( BidFileParser_Test );


void BidFileParser_Test::setUp() 
{
	const string filename = DEF_SYSCONFDIR "/example_bids1.xml";

	try
	{
		ptrBidFileParser = new BidFileParser(filename);
		idSource = new BidIdSource(1);

		// load the filter def list
		loadFieldDefs(&fieldDefs);
	
		// load the filter val list
		loadFieldVals(&fieldVals);

		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void BidFileParser_Test::tearDown() 
{
	saveDelete(ptrBidFileParser);
	saveDelete(idSource);
    saveDelete(ptrFieldParsers);
    saveDelete(ptrFieldValParser);
	
}

void BidFileParser_Test::testParser() 
{
	bidDB_t *new_bids = new bidDB_t();
		
	try
	{
		
		ptrBidFileParser->parse(&fieldDefs, 
							&fieldVals, 
							new_bids,
							idSource );
		
		CPPUNIT_ASSERT( new_bids->size() == 1 );
		
		saveDelete(new_bids);
		
	}
	catch (Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void BidFileParser_Test::loadFieldDefs(fieldDefList_t *fieldList)
{
	const string filename = DEF_SYSCONFDIR "/fielddef.xml";

	try
	{
		ptrFieldParsers = new FieldDefParser(filename);
		ptrFieldParsers->parse(fieldList);
		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}

}

void BidFileParser_Test::loadFieldVals(fieldValList_t *fieldValList)
{
	const string filename = DEF_SYSCONFDIR "/fieldval.xml";
	try
	{
		ptrFieldValParser = new FieldValParser(filename);
		ptrFieldValParser->parse(fieldValList);
				
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}

}
