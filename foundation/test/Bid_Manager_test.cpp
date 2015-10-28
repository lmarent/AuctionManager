/*
 * Test the Field_Value class.
 *
 * $Id: mnslp_fields.cpp 2014-11-28 10:16:00 amarentes $
 * $HeadURL: https://./test/mnslp_fields_test.cpp $
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
#include "BidManager.h"

using namespace auction;

class Bid_Manager_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( Bid_Manager_Test );

    CPPUNIT_TEST( testBidManager );
    CPPUNIT_TEST( testBidManager3 );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testBidManager();
	void testBidManager2();
	void testBidManager3();	
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);
	

  private:
    
    Bid *ptrBid1;
    Bid *ptrBid2;
    Bid *ptrBid3;
    Bid *ptrBid4;
    Bid *ptrBid5;
    
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;    
    BidFileParser *ptrBidFileParser;
    auto_ptr<EventScheduler>  evnt;
    BidManager *manager;
    
    
    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( Bid_Manager_Test );


void Bid_Manager_Test::setUp() 
{
		
	try
	{

		const string filename = "../../etc/example_bids1.xml";
		const string fieldname = DEF_SYSCONFDIR "/fielddef.xml";
		const string fieldValuename = DEF_SYSCONFDIR "/fieldval.xml";
		
		manager = new BidManager(fieldname,fieldValuename);

		// Parse the bid in file example_bids1.xml, it allocates the memory.
		bidDB_t *new_bids = manager->parseBids(filename);
		
		CPPUNIT_ASSERT( new_bids->size() == 1 );
		
		ptrBid1 = new Bid(*((*new_bids)[0]));

		auto_ptr<EventScheduler> _evnt(new EventScheduler());
        evnt = _evnt;
        
        saveDelete(new_bids);
		
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void Bid_Manager_Test::tearDown() 
{
	delete(ptrFieldParsers);
    delete(ptrFieldValParser);
	delete(ptrBidFileParser);
	delete(manager);
	
}

void Bid_Manager_Test::testBidManager() 
{

	try
	{

		ptrBid2 = new Bid(*ptrBid1);
		ptrBid3 = new Bid(*ptrBid1);
		ptrBid4 = new Bid(*ptrBid1);
		ptrBid5 = new Bid(*ptrBid1);

		manager->addBid(ptrBid1);
		
		CPPUNIT_ASSERT( manager->getNumBids() == 1 );

		ptrBid2->setBidSet("Agent2");
		manager->addBid(ptrBid2);
		CPPUNIT_ASSERT( manager->getNumBids() == 2 );
		manager->delBid(ptrBid1->getBidSet(), ptrBid1->getBidName(), evnt.get());
		
		Bid * bid2 = manager->getBid(ptrBid2->getBidSet(), 
										ptrBid2->getBidName());
		
		manager->delBid(bid2->getUId(), evnt.get());
		CPPUNIT_ASSERT( manager->getNumBids() == 0 );
		ptrBid3->setBidSet("Agent3");
		ptrBid4->setBidSet("Agent4");		
		manager->addBid(ptrBid3);
		manager->addBid(ptrBid4);
		CPPUNIT_ASSERT( manager->getNumBids() == 2 );
		
		bidDB_t in_bids = manager->getBids();
		manager->delBids(&in_bids, evnt.get());
		CPPUNIT_ASSERT( manager->getNumBids() == 0 );
	
		ptrBid5->setBidSet("Agent5");
		manager->addBid(ptrBid5);
		CPPUNIT_ASSERT( manager->getNumBids() == 1 );
		manager->delBids("Agent5", evnt.get());
		CPPUNIT_ASSERT( manager->getNumBids() == 0 );
			
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}


}



// This methods is implemented for testing auction indexes 
void Bid_Manager_Test::testBidManager3() 
{

	try
	{
	


		const string filename2 = "../../etc/example_bids2.xml";
		bidDB_t *new_bids2 = manager->parseBids(filename2);
		CPPUNIT_ASSERT( new_bids2->size() == 1 );
		ptrBid2 = new Bid(*((*new_bids2)[0]));
		saveDelete(new_bids2);
		

		const string filename3 = "../../etc/example_bids3.xml";
		bidDB_t *new_bids3 = manager->parseBids(filename3);
		CPPUNIT_ASSERT( new_bids3->size() == 1 );
		ptrBid3 = new Bid(*((*new_bids3)[0]));
		saveDelete(new_bids3);

		const string filename4 = "../../etc/example_bids4.xml";
		bidDB_t *new_bids4 = manager->parseBids(filename4);
		CPPUNIT_ASSERT( new_bids4->size() == 1 );
		ptrBid4 = new Bid(*((*new_bids4)[0]));
		saveDelete(new_bids4);

		const string filename5 = "../../etc/example_bids5.xml";
		bidDB_t *new_bids5 = manager->parseBids(filename5);
		CPPUNIT_ASSERT( new_bids5->size() == 1 );
		ptrBid5 = new Bid(*((*new_bids5)[0]));
		saveDelete(new_bids5);

		manager->addBid(ptrBid1);
		manager->addBid(ptrBid2);
		manager->addBid(ptrBid3);
		manager->addBid(ptrBid4);
		manager->addBid(ptrBid5);
		
		
		CPPUNIT_ASSERT( manager->getBids("general1","1").size() == 3);
		CPPUNIT_ASSERT( manager->getBids("general2","1").size() == 2);
		CPPUNIT_ASSERT( manager->getNumBids() == 5 );
				
		// Delete a bid 1, it should delete from the auction index containing 
		// auction general1.
		manager->delBid(ptrBid1->getUId(), evnt.get());
		CPPUNIT_ASSERT( manager->getBids("general1","1").size() == 2);
				
		// Delete a bid 2, it should delete from the auction index containing 
		// auction general1.
		manager->delBid(ptrBid2->getUId(), evnt.get());
		CPPUNIT_ASSERT( manager->getBids("general1","1").size() == 1);
				
		// Delete a bid 3, it should delete from the auction index containing 
		// auction general1.
		manager->delBid(ptrBid3->getUId(), evnt.get());
		CPPUNIT_ASSERT( manager->getBids("general1","1").size() == 0);
				
		CPPUNIT_ASSERT( manager->getBids("general2","1").size() == 2);
		
		
			
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}


}


void Bid_Manager_Test::loadFieldDefs(fieldDefList_t *fieldList)
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

void Bid_Manager_Test::loadFieldVals(fieldValList_t *fieldValList)
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




