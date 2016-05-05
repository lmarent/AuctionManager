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
		int domain = 0;
		
		ptrAuctionFileParser = new AuctionFileParser(domain, filename);
		
		ptrMAPIAuctionParser = new MAPIAuctionParser(domain);
						
		templates = new ipap_template_container();

		// load the filter def list
		loadFieldDefs(&fieldDefs);
		
		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void MAPIAuctionParser_Test::tearDown() 
{
	delete(ptrAuctionFileParser);
    delete(ptrFieldParsers);
	delete(templates);
	delete(ptrMAPIAuctionParser);
	
}

void MAPIAuctionParser_Test::testParser() 
{
	auctioningObjectDB_t *new_auctions = new auctioningObjectDB_t();
	
	auctioningObjectDB_t *new_auctions2 = new auctioningObjectDB_t();
		
	try
	{
		
		int domainId = 3;
		bool useIPV6 = false;
		string sAddressIPV4 = "192.168.2.11";
		string sAddressIPV6 = "0:0:0:0:0:0:0";
		uint16_t port = 12246;
		
		
		ptrAuctionFileParser->parse( &fieldDefs, new_auctions, templates );
		
		// Change the status of auctions in order to see that it send the correct 
		// information in the xml message.
		auctioningObjectDBIter_t iter;
		for (iter = new_auctions->begin(); iter != new_auctions->end(); ++iter)
		{
			(*iter)->setState(AO_ACTIVE);
		}
								
		// Build the message from auctions in the vector
				
		ipap_message * messages2 = ptrMAPIAuctionParser->
						get_ipap_message(&fieldDefs, new_auctions, 
										  templates, useIPV6, 
										  sAddressIPV4, sAddressIPV6, port );				
		
		anslp::msg::anslp_ipap_message msgb (*messages2);	
		
		anslp::msg::anslp_ipap_xml_message xmlMes;
				
		string sxmlMes = xmlMes.get_message(msgb);
		
		saveDelete(messages2);

		CPPUNIT_ASSERT( new_auctions->size() == 1 );

		saveDelete(new_auctions);
		
		list<int> templList2 = templates->get_template_list();
		
		templates->delete_all_templates();
		
		// Parse a XML with an auction
				
		const string resourceRequestFilename = "../../etc/ResponseRequestMessage.xml";
		
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
		throw e;
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
		throw e;
	}

}
