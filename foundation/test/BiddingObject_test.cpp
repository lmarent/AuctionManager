/*
 * Test the BiddingObject_test class.
 *
 * $Id: BiddingObject_test.cpp 2014-11-28 10:16:00 amarentes $
 * $HeadURL: https://./test/BiddingObject_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <config.h>
#include "BiddingObject.h"
#include "FieldValue.h"
#include "FieldValParser.h"
#include "FieldDefParser.h"
#include "BiddingObjectFileParser.h"

using namespace auction;

class BiddingObject_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( BiddingObject_Test );

    CPPUNIT_TEST( testBiddingObjects );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testBiddingObjects();
	void testFieldValues();
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);
	

  private:
    
    BiddingObject *ptrBid1;
    BiddingObject *ptrBid2;
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;    
    BiddingObjectFileParser *ptrBidFileParser;
        
    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( BiddingObject_Test );


void BiddingObject_Test::setUp() 
{
		
	try
	{

		int domain = 0;
		
		const string filename = "../../etc/example_bids1.xml";
		ptrBidFileParser = new BiddingObjectFileParser(domain, filename);

		// load the filter def list
		loadFieldDefs(&fieldDefs);
	
		// load the filter val list
		loadFieldVals(&fieldVals);

								
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void BiddingObject_Test::tearDown() 
{
	
	delete(ptrBidFileParser);
	delete(ptrFieldParsers);
	delete(ptrFieldValParser);	
	delete(ptrBid1);
	delete(ptrBid2);	
	
}

void BiddingObject_Test::testBiddingObjects() 
{
	try{
		biddingObjectDB_t *new_bids = new biddingObjectDB_t();

		ptrBidFileParser->parse(&fieldDefs, &fieldVals, new_bids );
			
		BiddingObject *copy = (*new_bids)[0];
				
		ptrBid1 = new BiddingObject(*copy);
		ptrBid2 = new BiddingObject(*((*new_bids)[0]));
				
		CPPUNIT_ASSERT( (ptrBid1->getInfo()).compare(((*new_bids)[0])->getInfo()) == 0 );
		
		CPPUNIT_ASSERT(	*ptrBid1 == *ptrBid2);
				
		for (int i = 0; i < new_bids->size() ; i++)
		{
			delete(((*new_bids)[i]));
		}
		new_bids->clear();
		delete new_bids;
		
	} catch (Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
	
	
}

void BiddingObject_Test::loadFieldDefs(fieldDefList_t *fieldList)
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

void BiddingObject_Test::loadFieldVals(fieldValList_t *fieldValList)
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




