/*
 * Test the MAPIAuctionParser_Test class.
 *
 * $Id: MAPIAuctionParser_Test.cpp 2015-09-22 14:25:00 amarentes $
 * $HeadURL: https://./test/MAPIAuctionParser_Test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "MAPIAuctionParser.h"
#include "AuctionIdSource.h"
#include "FieldDefParser.h"
#include "IpAp_message.h"

using namespace auction;

class MAPIAuctionParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( MAPIAuctionParser_Test );

	CPPUNIT_TEST( testParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void loadFieldDefs(fieldDefList_t *fieldList);
	void testParser();

  private:
    
    AuctionFileParser *ptrAuctionFileParser;
    AuctionIdSource *idSource;
    ipap_message *message;
    FieldDefParser *ptrFieldParsers;
    MAPIAuctionParser *ptrMAPIAuctionParser;
    
    //! filter definitions
    fieldDefList_t fieldDefs;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( MAPIAuctionParser_Test );


void MAPIAuctionParser_Test::setUp() 
{
	const string filename = DEF_SYSCONFDIR "/example_auctions2.xml";

	try
	{
		ptrAuctionFileParser = new AuctionFileParser(filename);
		
		ptrMAPIAuctionParser = new MAPIAuctionParser();
				
		idSource = new AuctionIdSource(1); // Unique.
		
		message = new ipap_message();

		// load the filter def list
		loadFieldDefs(&fieldDefs);
		
		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void MAPIAuctionParser_Test::tearDown() 
{
	delete(ptrAuctionFileParser);
	delete(idSource);
    delete(ptrFieldParsers);
	delete(message);
	delete(ptrMAPIAuctionParser);
	
}

void MAPIAuctionParser_Test::testParser() 
{
	auctionDB_t *new_auctions = new auctionDB_t();
		
	try
	{
		
		ptrAuctionFileParser->parse( &fieldDefs, new_auctions, idSource, message );
				
		cout << (*new_auctions)[0]->getInfo() << endl;
		
		// Build the message from auctions in the vector
		
		vector<ipap_message *> messages = ptrMAPIAuctionParser->
						get_ipap_messages(&fieldDefs, new_auctions);
		
		vector<ipap_message *>::iterator mesIterator;
		for (mesIterator = messages.begin(); mesIterator != messages.end(); ++mesIterator){
			ipap_message *mes = *mesIterator;
			saveDelete(mes);
		}
		
		CPPUNIT_ASSERT( new_auctions->size() == 1 );
		
	}
	catch (Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void MAPIAuctionParser_Test::loadFieldDefs(fieldDefList_t *fieldList)
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
