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
#include "anslp_ipap_xml_message.h"

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
    ipap_template_container *templates;
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
		
		templates = new ipap_template_container();

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
	delete(templates);
	delete(ptrMAPIAuctionParser);
	
}

void MAPIAuctionParser_Test::testParser() 
{
	auctionDB_t *new_auctions = new auctionDB_t();
	
	auctionDB_t *new_auctions2 = new auctionDB_t();
		
	try
	{
		
		ptrAuctionFileParser->parse( &fieldDefs, new_auctions, idSource, templates );
				
		cout << (*new_auctions)[0]->getInfo() << endl;
		
		// Build the message from auctions in the vector
		
		ipap_message * messages2 = ptrMAPIAuctionParser->
						get_ipap_message(&fieldDefs, new_auctions);

		saveDelete(messages2);

		CPPUNIT_ASSERT( new_auctions->size() == 1 );

		saveDelete(new_auctions);

		// Parse a XML with an auction
		
		const string resourceRequestFilename = DEF_SYSCONFDIR "/ResponseRequestMessage.xml";
		
		std::ifstream in(resourceRequestFilename.c_str());
		std::stringstream buffer;
		buffer << in.rdbuf();
		std::string test = buffer.str();
		
		anslp::msg::anslp_ipap_xml_message mess;
		anslp::msg::anslp_ipap_message *ipap_mes = mess.from_message(test);
		
		
		ptrMAPIAuctionParser->parse( &fieldDefs,
									 &(ipap_mes->ip_message),
									 new_auctions2,
									 templates );
		
		CPPUNIT_ASSERT( new_auctions2->size() == 1 );
		
		saveDelete(new_auctions2);
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
