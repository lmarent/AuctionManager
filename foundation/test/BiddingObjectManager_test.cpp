/*
 * Test the Bidding Object Manager class.
 *
 * $Id: BiddingObjectManager_test.cpp 2014-11-28 10:16:00 amarentes $
 * $HeadURL: https://./test/BiddingObjectManager_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "BiddingObject.h"
#include "FieldValue.h"
#include "FieldValParser.h"
#include "FieldDefParser.h"
#include "BiddingObjectFileParser.h"
#include "BiddingObjectManager.h"

using namespace auction;

class BiddingObjectManager_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( BiddingObjectManager_Test );

    CPPUNIT_TEST( testBiddingManagerManager );
    CPPUNIT_TEST( testBiddingObjectManager2 );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testBiddingManagerManager();
	void testBiddingObjectManager2();	
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);
	

  private:
    
    BiddingObject *ptrBid1;
    BiddingObject *ptrBid2;
    BiddingObject *ptrBid3;
    BiddingObject *ptrBid4;
    BiddingObject *ptrBid5;
    
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;    
    BiddingObjectFileParser *ptrBidFileParser;
    auto_ptr<EventScheduler>  evnt;
    BiddingObjectManager *manager;
    
    
    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( BiddingObjectManager_Test );


void BiddingObjectManager_Test::setUp() 
{
		
	try
	{

		int domain = 1;
		
		const string filename = "../../etc/example_bids1.xml";
		const string fieldname = DEF_SYSCONFDIR "/fielddef.xml";
		const string fieldValuename = DEF_SYSCONFDIR "/fieldval.xml";
		
		manager = new BiddingObjectManager(domain, fieldname, fieldValuename, "");

		// Parse the bidding objects in file example_bids1.xml, it allocates the memory.
		auctioningObjectDB_t *new_bids = manager->parseBiddingObjects(filename);
		
		CPPUNIT_ASSERT( new_bids->size() == 1 );
		
		BiddingObject *copy = dynamic_cast<BiddingObject *>((*new_bids)[0]);
		ptrBid1 = new BiddingObject(*copy);

		auto_ptr<EventScheduler> _evnt(new EventScheduler());
        evnt = _evnt;
        
        saveDelete(new_bids);
		
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void BiddingObjectManager_Test::tearDown() 
{
	delete(ptrFieldParsers);
    delete(ptrFieldValParser);
	delete(ptrBidFileParser);
	delete(manager);
	
}

void BiddingObjectManager_Test::testBiddingManagerManager() 
{

	try
	{

		ptrBid2 = new BiddingObject(*ptrBid1);
		ptrBid3 = new BiddingObject(*ptrBid1);
		ptrBid4 = new BiddingObject(*ptrBid1);
		ptrBid5 = new BiddingObject(*ptrBid1);

		manager->addBiddingObject(ptrBid1);
		
		CPPUNIT_ASSERT( manager->getNumAuctioningObjects() == 1 );

		ptrBid2->setSet("2");
		manager->addBiddingObject(ptrBid2);
		CPPUNIT_ASSERT( manager->getNumAuctioningObjects() == 2 );
		
		manager->delBiddingObject(ptrBid1->getSet(), 
								  ptrBid1->getName(), 
								  evnt.get());
		
		BiddingObject * bid2 = 
				dynamic_cast<BiddingObject *>(manager->getAuctioningObject(ptrBid2->getSet(), 
				    					  ptrBid2->getName()));
		
		manager->delBiddingObject(bid2->getUId(), evnt.get());
		CPPUNIT_ASSERT( manager->getNumAuctioningObjects() == 0 );
		ptrBid3->setSet("3");
		ptrBid4->setSet("4");		
		manager->addBiddingObject(ptrBid3);
		manager->addBiddingObject(ptrBid4);
		CPPUNIT_ASSERT( manager->getNumAuctioningObjects() == 2 );
		
		auctioningObjectDB_t in_bids = manager->getAuctioningObjects();
		manager->delAuctioningObjects(&in_bids, evnt.get());
		CPPUNIT_ASSERT( manager->getNumAuctioningObjects() == 0 );
	
		ptrBid5->setSet("5");
		manager->addBiddingObject(ptrBid5);
		
		CPPUNIT_ASSERT( manager->getNumAuctioningObjects() == 1 );
		manager->delBiddingObjects("5", evnt.get());
		CPPUNIT_ASSERT( manager->getNumAuctioningObjects() == 0 );
			
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}


}



// This methods is implemented for testing auction indexes 
void BiddingObjectManager_Test::testBiddingObjectManager2() 
{

	try
	{
	
		AuctioningObject *tmp;
		BiddingObject *tmpBO;
		
		//1.Bid2
		const string filename2 = "../../etc/example_bids2.xml";
		auctioningObjectDB_t *new_bids2 = manager->parseBiddingObjects(filename2);
		CPPUNIT_ASSERT( new_bids2->size() == 1 );
		tmp = (*new_bids2)[0];
		tmpBO = dynamic_cast<BiddingObject *>(tmp);
		ptrBid2 = new BiddingObject(*tmpBO);
		saveDelete(new_bids2);
		
		//2.Bid1
		const string filename3 = "../../etc/example_bids3.xml";
		auctioningObjectDB_t *new_bids3 = manager->parseBiddingObjects(filename3);
		CPPUNIT_ASSERT( new_bids3->size() == 1 );
		tmp = (*new_bids3)[0];
		tmpBO = dynamic_cast<BiddingObject *>(tmp);
		ptrBid3 = new BiddingObject(*tmpBO);
		saveDelete(new_bids3);

		//3.Bid1
		const string filename4 = "../../etc/example_bids4.xml";
		auctioningObjectDB_t *new_bids4 = manager->parseBiddingObjects(filename4);
		CPPUNIT_ASSERT( new_bids4->size() == 1 );
		tmp = (*new_bids4)[0];
		tmpBO = dynamic_cast<BiddingObject *>(tmp);
		ptrBid4 = new BiddingObject(*tmpBO);
		saveDelete(new_bids4);

		
		//4.Bid1
		const string filename5 = "../../etc/example_bids5.xml";
		auctioningObjectDB_t *new_bids5 = manager->parseBiddingObjects(filename5);
		CPPUNIT_ASSERT( new_bids5->size() == 1 );
		tmp = (*new_bids5)[0];
		tmpBO = dynamic_cast<BiddingObject *>(tmp);
		ptrBid5 = new BiddingObject(*tmpBO);
		saveDelete(new_bids5);

		
		manager->addBiddingObject(ptrBid1);
		manager->addBiddingObject(ptrBid2);
		manager->addBiddingObject(ptrBid3);
		manager->addBiddingObject(ptrBid4);
		manager->addBiddingObject(ptrBid5);
		
		CPPUNIT_ASSERT( manager->getBiddingObjects("1","1").size() == 3);
		CPPUNIT_ASSERT( manager->getBiddingObjects("2","1").size() == 2);
		CPPUNIT_ASSERT( manager->getNumAuctioningObjects() == 5 );
				
		// Delete a BiddingObject 1, it should delete from the auction index containing 
		// bid 1.Bid1
		manager->delBiddingObject(ptrBid1->getUId(), evnt.get());
		CPPUNIT_ASSERT( manager->getBiddingObjects("1","1").size() == 2);
				
		// Delete a BiddingObject 2, it should delete from the auction index containing 
		// auction general1.
		manager->delBiddingObject(ptrBid2->getUId(), evnt.get());
		CPPUNIT_ASSERT( manager->getBiddingObjects("1","1").size() == 1);
				
		// Delete a BiddingObject 3, it should delete from the auction index containing 
		// auction general1.
		manager->delBiddingObject(ptrBid3->getUId(), evnt.get());
		CPPUNIT_ASSERT( manager->getBiddingObjects("1","1").size() == 0);
				
		CPPUNIT_ASSERT( manager->getBiddingObjects("2","1").size() == 2);
		
		
			
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}


}


void BiddingObjectManager_Test::loadFieldDefs(fieldDefList_t *fieldList)
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

void BiddingObjectManager_Test::loadFieldVals(fieldValList_t *fieldValList)
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




