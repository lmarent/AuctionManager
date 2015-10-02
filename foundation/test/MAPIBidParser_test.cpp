/*
 * Test the MAPIBidParser_Test class.
 *
 * $Id: MAPIBidParser_Test.cpp 2015-09-30 10:25:00 amarentes $
 * $HeadURL: https://./test/MAPIBidParser_Test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "Bid.h"
#include "FieldValue.h"
#include "FieldValParser.h"
#include "FieldDefParser.h"
#include "BidIdSource.h"
#include "BidFileParser.h"
#include "MAPIBidParser.h"


using namespace auction;

class MAPIBidParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( MAPIBidParser_Test );

    CPPUNIT_TEST( testMAPIBidParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testMAPIBidParser();
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);
	

  private:
    
    Bid *ptrBid1;
    Bid *ptrBid2;
    Bid *ptrBid3;
    
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;    
    BidFileParser *ptrBidFileParser;
    MAPIBidParser * ptrMAPIBidParser;    
    
    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( MAPIBidParser_Test );


void MAPIBidParser_Test::setUp() 
{
		
	try
	{

		const string fieldname = DEF_SYSCONFDIR "/fielddef.xml";
		const string fieldValuename = DEF_SYSCONFDIR "/fieldval.xml";
		
		// load the filter def list
		loadFieldDefs(&fieldDefs);
	
		// load the filter val list
		loadFieldVals(&fieldVals);

		BidIdSource *idSource = new BidIdSource(1);

		const string filename1 = DEF_SYSCONFDIR "/example_bids1.xml";
		ptrBidFileParser = new BidFileParser(filename1);
		
		bidDB_t *new_bids1 = new bidDB_t();		
		ptrBidFileParser->parse(&fieldDefs, 
							&fieldVals, 
							new_bids1,
							idSource );
		ptrBid1 = new Bid(*((*new_bids1)[0]));
		saveDelete(new_bids1);
		saveDelete(ptrBidFileParser);
		
		const string filename2 = DEF_SYSCONFDIR "/example_bids2.xml";	
		ptrBidFileParser = new BidFileParser(filename2);
		
		bidDB_t *new_bids2 = new bidDB_t();		
		ptrBidFileParser->parse(&fieldDefs, &fieldVals, new_bids2, idSource );
		
		ptrBid2 = new Bid(*((*new_bids2)[0]));
		saveDelete(new_bids2);
		saveDelete(ptrBidFileParser);

		const string filename3 = DEF_SYSCONFDIR "/example_bids3.xml";	
		ptrBidFileParser = new BidFileParser(filename3);
		
		bidDB_t *new_bids3 = new bidDB_t();		
		ptrBidFileParser->parse(&fieldDefs, &fieldVals, new_bids3, idSource );
		ptrBid3 = new Bid(*((*new_bids3)[0]));
		saveDelete(new_bids3);
		saveDelete(ptrBidFileParser);
				
		ptrMAPIBidParser = new MAPIBidParser();
		        		
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void MAPIBidParser_Test::tearDown() 
{
	saveDelete(ptrFieldParsers);
    saveDelete(ptrFieldValParser);
	saveDelete(ptrMAPIBidParser);
	saveDelete(ptrBid1);
	saveDelete(ptrBid2);
	saveDelete(ptrBid3);
	
}

void MAPIBidParser_Test::testMAPIBidParser() 
{

	try
	{
		
		Bid *ptrBidTmp;
		BidIdSource *idSource = new BidIdSource(1);
		bidDB_t *bids = new bidDB_t();	
		bids->push_back(ptrBid1);
		bids->push_back(ptrBid2);
		bids->push_back(ptrBid3);
		
		vector<ipap_message *> vct_message = ptrMAPIBidParser->get_ipap_messages(&fieldDefs, bids);
		
		CPPUNIT_ASSERT( vct_message.size() == 3 );
		
		ipap_message *output = new ipap_message(); 
		
		
		bidDB_t *bids2 = new bidDB_t();	
		
		ptrMAPIBidParser->parse(&fieldDefs, &fieldVals, vct_message[0],
								bids2, idSource, output );
		
		CPPUNIT_ASSERT( bids2->size() == 1 );
		
		ptrBidTmp = (*bids2)[0];
				
		CPPUNIT_ASSERT( *ptrBidTmp == *ptrBid1 );
		saveDelete(bids2);

		bidDB_t *bids3 = new bidDB_t();	
		
		ptrMAPIBidParser->parse(&fieldDefs, &fieldVals, vct_message[1],
								bids3, idSource, output );
		
		CPPUNIT_ASSERT( bids3->size() == 1 );
		
		ptrBidTmp = (*bids3)[0];
		CPPUNIT_ASSERT( *ptrBidTmp == *ptrBid2 );
		saveDelete(bids3);


		bidDB_t *bids4 = new bidDB_t();	
		
		ptrMAPIBidParser->parse(&fieldDefs, &fieldVals, vct_message[2],
								bids4, idSource, output );
		
		CPPUNIT_ASSERT( bids4->size() == 1 );
		
		ptrBidTmp = (*bids4)[0];
		CPPUNIT_ASSERT( *ptrBidTmp == *ptrBid3 );
		saveDelete(bids4);
		
			
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		CPPUNIT_ASSERT( "An anormal exception was generated" == "0" );
	}


}


void MAPIBidParser_Test::loadFieldDefs(fieldDefList_t *fieldList)
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

void MAPIBidParser_Test::loadFieldVals(fieldValList_t *fieldValList)
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




