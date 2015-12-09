/*
 * Test the MAPIBiddingObjectParser_Test class.
 *
 * $Id: MAPIBiddingObjectParser_Test.cpp 2015-09-30 10:25:00 amarentes $
 * $HeadURL: https://./test/MAPIBiddingObjectParser_Test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "BiddingObject.h"
#include "FieldValue.h"
#include "FieldValParser.h"
#include "FieldDefParser.h"
#include "BiddingObjectFileParser.h"
#include "MAPIBiddingObjectParser.h"
#include "anslp_ipap_message.h"
#include "anslp_ipap_xml_message.h"



using namespace auction;

class MAPIBiddingObjectParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( MAPIBiddingObjectParser_Test );

    CPPUNIT_TEST( testMAPIBiddingObjectParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testMAPIBiddingObjectParser();
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);
	auctionDB_t * loadAuctions();
	biddingObjectDB_t * loadBidsFromFile();
	

  private:
        
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;    
    MAPIBiddingObjectParser * ptrMAPIBidParser;    
    
    ipap_template_container *templates;
    
    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( MAPIBiddingObjectParser_Test );


auctionDB_t * 
MAPIBiddingObjectParser_Test::loadAuctions()
{
	
	const string filename = "../../etc/example_auctions2.xml";
	auctionDB_t *new_auctions = NULL;
	AuctionFileParser *ptrAuctionFileParser = NULL; 

	try {
		
		int domain = 4;
		
		new_auctions = new auctionDB_t();
		ptrAuctionFileParser = new AuctionFileParser(domain, filename);
		ptrAuctionFileParser->parse( &fieldDefs, new_auctions, templates );
		
		saveDelete(ptrAuctionFileParser);
		
		return new_auctions;

	} catch (Error &e){
		
		if (ptrAuctionFileParser!= NULL)
			saveDelete(ptrAuctionFileParser);
			
		for(auctionDBIter_t i=new_auctions->begin(); i != new_auctions->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_auctions);
		
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

biddingObjectDB_t * 
MAPIBiddingObjectParser_Test::loadBidsFromFile()
{

    biddingObjectDB_t *new_bids = NULL;
    BiddingObjectFileParser *ptrBidFileParser = NULL; 
	int domain = 3;
	
	try {
		
		
		new_bids = new biddingObjectDB_t();

		const string filename1 =  "../../etc/example_bids1.xml";
		ptrBidFileParser = new BiddingObjectFileParser(domain,filename1);
			
		biddingObjectDB_t *new_bids1 = new biddingObjectDB_t();		
		ptrBidFileParser->parse(&fieldDefs, &fieldVals, new_bids1 );
		new_bids->push_back((*new_bids1)[0]);
		
		saveDelete(new_bids1);
		saveDelete(ptrBidFileParser);
			
		const string filename2 = "../../etc/example_bids2.xml";	
		ptrBidFileParser = new BiddingObjectFileParser(domain,filename2);
			
		biddingObjectDB_t *new_bids2 = new biddingObjectDB_t();		
		ptrBidFileParser->parse(&fieldDefs, &fieldVals, new_bids2 );
		new_bids->push_back((*new_bids2)[0]);	
		saveDelete(new_bids2);
		saveDelete(ptrBidFileParser);

		const string filename3 = "../../etc/example_bids3.xml";	
		ptrBidFileParser = new BiddingObjectFileParser(domain,filename3);
			
		biddingObjectDB_t *new_bids3 = new biddingObjectDB_t();		
		ptrBidFileParser->parse(&fieldDefs, &fieldVals, new_bids3 );
		new_bids->push_back((*new_bids3)[0]);	
		
		saveDelete(new_bids3);
		saveDelete(ptrBidFileParser);
		
		return new_bids;
		
	} catch (Error &e) {
		if (ptrBidFileParser!= NULL)
			saveDelete(ptrBidFileParser);
		
		for(biddingObjectDBIter_t i=new_bids->begin(); i != new_bids->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_bids);

		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;		
	}
	
}

void MAPIBiddingObjectParser_Test::setUp() 
{
		
	try
	{

		int domain = 5;
		
		const string fieldname = DEF_SYSCONFDIR "/fielddef.xml";
		const string fieldValuename = DEF_SYSCONFDIR "/fieldval.xml";
		
		// load the filter def list
		loadFieldDefs(&fieldDefs);
	
		// load the filter val list
		loadFieldVals(&fieldVals);
		
		// templates
		templates = new ipap_template_container();
				
		ptrMAPIBidParser = new MAPIBiddingObjectParser(domain);
		        		
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void MAPIBiddingObjectParser_Test::tearDown() 
{
	saveDelete(ptrFieldParsers);
    saveDelete(ptrFieldValParser);
	saveDelete(ptrMAPIBidParser);
	saveDelete(templates);
	
}

void MAPIBiddingObjectParser_Test::testMAPIBiddingObjectParser() 
{

	try
	{
		
		BiddingObject *ptrBidTmp1, *ptrBidTmp2, *ptrBidTmp3, *ptrBidTmp4, *ptrBidTmp5, *ptrBidTmp6;

		auctionDB_t * auctions = loadAuctions();
		Auction *auction = (*auctions)[0];
		biddingObjectDB_t *bids = loadBidsFromFile();
		
		BiddingObject *object1 = (*bids)[0];
		BiddingObject *object2 = (*bids)[1];
		BiddingObject *object3 = (*bids)[2];
		
		CPPUNIT_ASSERT( bids->size() == 3 );
				
		ipap_message * message1 = ptrMAPIBidParser->get_ipap_message(&fieldDefs, object1, auction, templates);
		ipap_message * message2 = ptrMAPIBidParser->get_ipap_message(&fieldDefs, object2, auction, templates);
		ipap_message * message3 = ptrMAPIBidParser->get_ipap_message(&fieldDefs, object3, auction, templates);
		
		anslp::msg::anslp_ipap_message mes(*message1);
		
		anslp::msg::anslp_ipap_xml_message xmlmes;
	
		string xmlMessage = xmlmes.get_message(mes);	
	
		// Activate to see the message 
		cout << "Message:" << xmlMessage << endl;
								
		biddingObjectDB_t *bids2 = new biddingObjectDB_t();	
		
		ptrMAPIBidParser->parse(&fieldDefs, &fieldVals, message1, bids2, templates );
				
		cout << "Bid size:" << bids2->size() << endl;
		
		CPPUNIT_ASSERT( bids2->size() == 1 );
		
		ptrBidTmp2 = (*bids2)[0];
				
		CPPUNIT_ASSERT( *object1 == *ptrBidTmp2 );


		// Delete bids and auctions
		for(auctionDBIter_t i=auctions->begin(); i != auctions->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(auctions);


		for(biddingObjectDBIter_t i=bids->begin(); i != bids->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(bids);

		for(biddingObjectDBIter_t i=bids2->begin(); i != bids2->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(bids2);		
			
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}


}


void MAPIBiddingObjectParser_Test::loadFieldDefs(fieldDefList_t *fieldList)
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

void MAPIBiddingObjectParser_Test::loadFieldVals(fieldValList_t *fieldValList)
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




