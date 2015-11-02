/*
 * Test the Auction_test class.
 *
 * $Id: Auction_test.cpp 2014-11-28 10:16:00 amarentes $
 * $HeadURL: https://./test/Auction_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <config.h>
#include "Auction.h"
#include "FieldValue.h"
#include "FieldValParser.h"
#include "FieldDefParser.h"
#include "AuctionFileParser.h"

using namespace auction;

class Auction_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( Auction_Test );

    CPPUNIT_TEST( testAuctions );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testAuctions();
	void testFieldValues();
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);
	

  private:
    
    Auction *ptrAuction1;
    Auction *ptrAuction2;
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;    
    AuctionFileParser *ptrActFileParser;
    ipap_template_container* templates;
        
    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( Auction_Test );


void Auction_Test::setUp() 
{
		
	try
	{

		int domain = 0;
		
		const string filename = "../../etc/example_auctions1.xml";
		ptrActFileParser = new AuctionFileParser(domain, filename);

		// load the filter def list
		loadFieldDefs(&fieldDefs);
	
		// load the filter val list
		loadFieldVals(&fieldVals);
		
		templates = new ipap_template_container();
								
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void Auction_Test::tearDown() 
{
	
	saveDelete(ptrActFileParser);
	saveDelete(ptrFieldParsers);
	saveDelete(ptrFieldValParser);	
	saveDelete(ptrAuction1);
	saveDelete(ptrAuction2);
	saveDelete(templates);	
	
}

void Auction_Test::testAuctions() 
{
	try{
		auctionDB_t *new_acts = new auctionDB_t();

		ptrActFileParser->parse(&fieldDefs, new_acts, templates );
			
		Auction *copy = (*new_acts)[0];
				
		ptrAuction1 = new Auction(*copy);
		ptrAuction2 = new Auction(*((*new_acts)[0]));
				
		CPPUNIT_ASSERT( (ptrAuction1->getInfo()).compare(((*new_acts)[0])->getInfo()) == 0 );
						
		for (int i = 0; i < new_acts->size() ; i++)
		{
			delete(((*new_acts)[i]));
		}
		
		new_acts->clear();
		delete new_acts;
		
	} catch (Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
	
	
}

void Auction_Test::loadFieldDefs(fieldDefList_t *fieldList)
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

void Auction_Test::loadFieldVals(fieldValList_t *fieldValList)
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
