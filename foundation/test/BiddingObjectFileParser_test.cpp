/*
 * Test the BiddingObjectFileParser class.
 *
 * $Id: BiddingObjectFileParser_Test.cpp 2015-07-24 15:50:00 amarentes $
 * $HeadURL: https://./test/BiddingObjectFileParser_Test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "BiddingObjectManager.h"
#include "BiddingObjectFileParser.h"
#include "FieldValParser.h"
#include "FieldDefParser.h"

using namespace auction;

class BiddingObjectFileParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( BiddingObjectFileParser_Test );

	CPPUNIT_TEST( testParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);

	void testParser();

  private:
    
    BiddingObjectFileParser *ptrBidFileParser;
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;

    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( BiddingObjectFileParser_Test );


void BiddingObjectFileParser_Test::setUp() 
{
	const string filename = "../../etc/example_bids1.xml";

	try
	{
		int domain = 0; 
		
		ptrBidFileParser = new BiddingObjectFileParser(domain, filename);

		// load the filter def list
		loadFieldDefs(&fieldDefs);
	
		// load the filter val list
		loadFieldVals(&fieldVals);

		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void BiddingObjectFileParser_Test::tearDown() 
{
	saveDelete(ptrBidFileParser);
    saveDelete(ptrFieldParsers);
    saveDelete(ptrFieldValParser);
	
}

void BiddingObjectFileParser_Test::testParser() 
{
	biddingObjectDB_t *newBiddingObjects = new biddingObjectDB_t();
		
	try
	{
		
		ptrBidFileParser->parse(&fieldDefs, &fieldVals, newBiddingObjects );
		
		CPPUNIT_ASSERT( newBiddingObjects->size() == 1 );
		
		saveDelete(newBiddingObjects);
		
	}
	catch (Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void BiddingObjectFileParser_Test::loadFieldDefs(fieldDefList_t *fieldList)
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

void BiddingObjectFileParser_Test::loadFieldVals(fieldValList_t *fieldValList)
{
	const string filename = DEF_SYSCONFDIR "/fieldval.xml";
	try
	{
		ptrFieldValParser = new FieldValParser(filename);
		ptrFieldValParser->parse(fieldValList);
				
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}

}
