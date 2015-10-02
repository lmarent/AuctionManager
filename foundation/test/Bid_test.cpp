/*
 * Test the Field_Value class.
 *
 * $Id: mnslp_fields.cpp 2014-11-28 10:16:00 amarentes $
 * $HeadURL: https://./test/mnslp_fields_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <config.h>
#include "Bid.h"
#include "FieldValue.h"
#include "FieldValParser.h"
#include "FieldDefParser.h"
#include "BidIdSource.h"
#include "BidFileParser.h"

using namespace auction;

class Bid_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( Bid_Test );

    CPPUNIT_TEST( testBids );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testBids();
	void testFieldValues();
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);
	

  private:
    
    Bid *ptrBid1;
    Bid *ptrBid2;
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;    
    BidFileParser *ptrBidFileParser;
    BidIdSource *idSource;
    
    
    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( Bid_Test );


void Bid_Test::setUp() 
{
		
	try
	{

		const string filename = DEF_SYSCONFDIR "/example_bids1.xml";
		ptrBidFileParser = new BidFileParser(filename);
		idSource = new BidIdSource(1);

		// load the filter def list
		loadFieldDefs(&fieldDefs);
	
		// load the filter val list
		loadFieldVals(&fieldVals);

		bidDB_t *new_bids = new bidDB_t();

		ptrBidFileParser->parse(&fieldDefs, 
							&fieldVals, 
							new_bids,
							idSource );
			
		Bid *copy = (*new_bids)[0];
				
		ptrBid1 = new Bid(*copy);
		ptrBid2 = new Bid(*((*new_bids)[0]));
				
		CPPUNIT_ASSERT( (ptrBid1->getInfo()).compare(((*new_bids)[0])->getInfo()) == 0 );
		
		CPPUNIT_ASSERT(	*ptrBid1 == *ptrBid2);
				
		for (int i = 0; i < new_bids->size() ; i++)
		{
			delete(((*new_bids)[i]));
		}
		new_bids->clear();
		delete new_bids;
								
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void Bid_Test::tearDown() 
{
	
	delete(idSource);
	delete(ptrBidFileParser);
	delete(ptrFieldParsers);
	delete(ptrFieldValParser);	
	delete(ptrBid1);
	delete(ptrBid2);	
	
}

void Bid_Test::testBids() 
{

}

void Bid_Test::loadFieldDefs(fieldDefList_t *fieldList)
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

void Bid_Test::loadFieldVals(fieldValList_t *fieldValList)
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




