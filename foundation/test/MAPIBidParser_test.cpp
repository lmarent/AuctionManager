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
#include "anslp_ipap_message.h"
#include "anslp_ipap_xml_message.h"



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
	auctionDB_t * loadAuctions();
	bidDB_t * loadBidsFromFile();
	

  private:
        
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;    
    MAPIBidParser * ptrMAPIBidParser;    
    
    ipap_template_container *templates;
    
    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( MAPIBidParser_Test );


auctionDB_t * 
MAPIBidParser_Test::loadAuctions()
{
	
	const string filename = DEF_SYSCONFDIR "/example_auctions2.xml";
	auctionDB_t *new_auctions = NULL;
	AuctionFileParser *ptrAuctionFileParser = NULL; 
	AuctionIdSource *idSource = NULL; 
	try {
		
		new_auctions = new auctionDB_t();
		ptrAuctionFileParser = new AuctionFileParser(filename);
		idSource = new AuctionIdSource(1); // Unique.
		ptrAuctionFileParser->parse( &fieldDefs, new_auctions, idSource, templates );
		
		saveDelete(ptrAuctionFileParser);
		saveDelete(idSource);
		
		return new_auctions;

	} catch (Error &e){
		
		if (ptrAuctionFileParser!= NULL)
			saveDelete(ptrAuctionFileParser);
			
		if (idSource != NULL)
			saveDelete(idSource);
		
		for(auctionDBIter_t i=new_auctions->begin(); i != new_auctions->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_auctions);
		
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

bidDB_t * 
MAPIBidParser_Test::loadBidsFromFile()
{

    bidDB_t *new_bids = NULL;
    BidFileParser *ptrBidFileParser = NULL; 
    BidIdSource *idSource = NULL; 
	
	try {
		
		new_bids = new bidDB_t();
		idSource = new BidIdSource(1); // Unique.
		
		const string filename1 =  "../../etc/example_bids1.xml";
		ptrBidFileParser = new BidFileParser(filename1);
			
		bidDB_t *new_bids1 = new bidDB_t();		
		ptrBidFileParser->parse(&fieldDefs, &fieldVals, new_bids1,idSource );
		new_bids->push_back((*new_bids1)[0]);
		
		saveDelete(new_bids1);
		saveDelete(ptrBidFileParser);
			
		const string filename2 = "../../etc/example_bids2.xml";	
		ptrBidFileParser = new BidFileParser(filename2);
			
		bidDB_t *new_bids2 = new bidDB_t();		
		ptrBidFileParser->parse(&fieldDefs, &fieldVals, new_bids2, idSource );
		new_bids->push_back((*new_bids2)[0]);	
		saveDelete(new_bids2);
		saveDelete(ptrBidFileParser);

		const string filename3 = "../../etc/example_bids3.xml";	
		ptrBidFileParser = new BidFileParser(filename3);
			
		bidDB_t *new_bids3 = new bidDB_t();		
		ptrBidFileParser->parse(&fieldDefs, &fieldVals, new_bids3, idSource );
		new_bids->push_back((*new_bids3)[0]);	
		
		saveDelete(new_bids3);
		saveDelete(ptrBidFileParser);
		saveDelete(idSource);
		
		return new_bids;
		
	} catch (Error &e) {
		if (ptrBidFileParser!= NULL)
			saveDelete(ptrBidFileParser);
		
		if (idSource != NULL)
			saveDelete(idSource);

		for(bidDBIter_t i=new_bids->begin(); i != new_bids->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_bids);

		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;		
	}
	
}

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
		
		// templates
		templates = new ipap_template_container();
				
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
	saveDelete(templates);
	
}

void MAPIBidParser_Test::testMAPIBidParser() 
{

	try
	{
		
		int domain = 7;
		Bid *ptrBidTmp1, *ptrBidTmp2, *ptrBidTmp3, *ptrBidTmp4, *ptrBidTmp5, *ptrBidTmp6;

		auctionDB_t * auctions = loadAuctions();
		bidDB_t *bids = loadBidsFromFile();
		
		cout << "number of templates:" << templates->get_num_templates() << endl;
		
		ipap_message * message = ptrMAPIBidParser->get_ipap_message(&fieldDefs, bids, auctions, templates, domain );
		
		anslp::msg::anslp_ipap_message mes(*message);
		
		anslp::msg::anslp_ipap_xml_message xmlmes;
	
		string xmlMessage = xmlmes.get_message(mes);	
		
		cout << "message:" << xmlMessage << endl;
						
		bidDB_t *bids2 = new bidDB_t();	
		
		ptrMAPIBidParser->parse(&fieldDefs, &fieldVals, message, bids2, templates );
		
		cout << "Bids able to read:" << bids2->size() << endl;
		
		CPPUNIT_ASSERT( bids2->size() == 3 );
		
		
		ptrBidTmp1 = (*bids)[0];
		ptrBidTmp2 = (*bids2)[0];
				
		CPPUNIT_ASSERT( *ptrBidTmp1 == *ptrBidTmp2 );

		ptrBidTmp3 = (*bids)[1];
		ptrBidTmp4 = (*bids2)[1];
	
		CPPUNIT_ASSERT( *ptrBidTmp3 == *ptrBidTmp4 );
		
		ptrBidTmp5 = (*bids)[2];
		ptrBidTmp6 = (*bids2)[2];

		CPPUNIT_ASSERT( *ptrBidTmp5 == *ptrBidTmp6 );


		// Delete bids and auctions
		for(auctionDBIter_t i=auctions->begin(); i != auctions->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(auctions);


		for(bidDBIter_t i=bids->begin(); i != bids->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(bids);

		for(bidDBIter_t i=bids2->begin(); i != bids2->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(bids2);		
			
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
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
		throw e;
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
		throw e;
	}

}




