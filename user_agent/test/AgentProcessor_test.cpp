/*
 * Test the AgentProcessor_test class.
 *
 * $Id: AgentProcessor_test.cpp 2015-11-06 10:28:00 amarentes $
 * $HeadURL: https://./test/AgentProcessor_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "AgentProcessor.h"
#include "Constants.h"
#include "ConstantsAgent.h"
#include "ConfigManager.h"
#include "EventScheduler.h"
#include "AuctionManager.h"

using namespace auction;

class AgentProcessor_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( AgentProcessor_Test );

	CPPUNIT_TEST( testBasicInsertDelete );
	CPPUNIT_TEST_EXCEPTION( testException2, Error );
	CPPUNIT_TEST( testBasicExecution );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void testBasicInsertDelete();
	void testException2();
	void testBasicExecution();

  private:

	  Auction *ptrAct1;
      Auction *ptrAct2;
      Auction *ptrAct3;

	  ConfigManager *configManagerPtr;
	  AuctionManager *auctionManagerPtr;
	  AgentProcessor *agntProcessorPtr;
      auto_ptr<EventScheduler>  evnt;
      
      auction::fieldList_t *fields; 
	  
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( AgentProcessor_Test );


void AgentProcessor_Test::setUp() 
{
		
	try
	{
			
		const string configDTD = DEF_SYSCONFDIR "/netagnt.conf.dtd";
		const string configFileName = AGNT_DEFAULT_CONFIG_FILE;
		configManagerPtr = new ConfigManager(configDTD, configFileName, "./netagent");
		auctioningObjectDB_t * auctions = NULL;

		auto_ptr<EventScheduler> _evnt(new EventScheduler());
        evnt = _evnt;		
		
		ipap_template_container *templContainer = new ipap_template_container();
		
		int domainAuct = 8;
		int ownDomain = 6;
		
		auctionManagerPtr = new AuctionManager(domainAuct, 
								configManagerPtr->getValue("FieldDefFile", "MAIN") , 
								configManagerPtr->getValue("FilterConstFile", "MAIN"),
								false );

		string filenameAuctions = "../../etc/example_auctions2.xml";
		
		auctions = auctionManagerPtr->parseAuctions(filenameAuctions, templContainer);
				
		ptrAct1 = dynamic_cast<Auction *>((*auctions)[0]);

		auctionManagerPtr->addAuctioningObjects(auctions, evnt.get());
		
		saveDelete(auctions);

		filenameAuctions = "../../etc/example_auctions1.xml";
		
		auctions = auctionManagerPtr->parseAuctions(filenameAuctions, templContainer);

		auctionManagerPtr->addAuctioningObjects(auctions, evnt.get());
				
		ptrAct2 = dynamic_cast<Auction *>((*auctions)[0]);
		
		saveDelete(auctions);
		
		CPPUNIT_ASSERT( auctionManagerPtr->getNumAuctioningObjects() == 3 );
		
		agntProcessorPtr = new AgentProcessor(ownDomain,
											  configManagerPtr, 
											  configManagerPtr->getValue("FieldDefFile", "MAIN"),
											  configManagerPtr->getValue("FilterConstFile", "MAIN"),
										      configManagerPtr->isTrue("Thread","AGENT_PROCESSOR"));

		fields = new auction::fieldList_t();
		
		field_t field1;
		field_t field2;
		field_t field3;
		field_t field4;
		
		
		auction::fieldDefListIter_t iter; 
		iter = auctionManagerPtr->getFieldDefs()->find("quantity");
		if (iter != auctionManagerPtr->getFieldDefs()->end()){
			field1.name = iter->second.name;
			field1.len = iter->second.len;
			field1.type = iter->second.type;
			string fvalue1 = "2";
			field1.parseFieldValue(fvalue1);
		} else {
			throw Error("field quantity sot found");
		}

		iter = auctionManagerPtr->getFieldDefs()->find("maxvalue");
		if (iter != auctionManagerPtr->getFieldDefs()->end()){
			field2.name = iter->second.name;
			field2.len = iter->second.len;
			field2.type = iter->second.type;
			string fvalue2 = "0.012";
			field2.parseFieldValue(fvalue2);
		} else {
			throw Error("field unitprice sot found");
		}
		
		iter = auctionManagerPtr->getFieldDefs()->find("totalbudget");
		if (iter != auctionManagerPtr->getFieldDefs()->end()){
			field3.name = iter->second.name;
			field3.len = iter->second.len;
			field3.type = iter->second.type;
			string fvalue3 = "0.08";
			field3.parseFieldValue(fvalue3);
		} else {
			throw Error("field unitbudget sot found");
		}


		fields->push_back(field1);
		fields->push_back(field2);
		fields->push_back(field3);

						
	}
	catch(Error &e){
		cout << "Error:" << e.getError() << endl << flush;
		throw e;
	}

	
}

void AgentProcessor_Test::tearDown() 
{
	
	if (auctionManagerPtr != NULL)
		saveDelete(auctionManagerPtr);
	
	if (agntProcessorPtr != NULL)
		saveDelete(agntProcessorPtr); //! This delete need the config Manager ptr, so 
							 //! first we need to delete this pointer.
	if (fields != NULL)
		saveDelete(fields);
	
	if (configManagerPtr != NULL)
		saveDelete(configManagerPtr);
		
	evnt.reset();
		
}

void AgentProcessor_Test::testBasicInsertDelete() 
{

	try
	{
		
		int index = 0;
		// Build a new request process.
		string sessionId = "session1";
		index = agntProcessorPtr->addRequest( sessionId, fields, ptrAct1, ptrAct1->getStart(), ptrAct1->getStop() );
		
		// Add a new auction to the request process.
		agntProcessorPtr->addAuctionRequest(index, ptrAct2 );
		
		CPPUNIT_ASSERT( agntProcessorPtr->getNumRequestProcess() == 1 );
		
		CPPUNIT_ASSERT(  ((agntProcessorPtr->begin())->second).getAuctions()->size() == 2 );
		
		// Delete the auction from the request process.
		agntProcessorPtr->delAuctionRequest(index, ptrAct2 );
		
		CPPUNIT_ASSERT( agntProcessorPtr->getNumRequestProcess() == 1 );
		
		CPPUNIT_ASSERT(  ((agntProcessorPtr->begin())->second).getAuctions()->size() == 1 );
		
		agntProcessorPtr->delRequest(index);
		
		CPPUNIT_ASSERT( agntProcessorPtr->getNumRequestProcess() == 0 );
		
		
		
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
	
}

void AgentProcessor_Test::testException2()
{

	// Test to insert the same auction twice.
		
		int index = 0;
		// Build a new request process.
		string sessionId = "session1";
		index = agntProcessorPtr->addRequest( sessionId, fields, ptrAct1, ptrAct1->getStart(), ptrAct1->getStop() );
		
		// Add a new auction to the request process.
		agntProcessorPtr->addAuctionRequest(index, ptrAct1 );
}

void AgentProcessor_Test::testBasicExecution()
{

	try{
		int index = 0;
		// Build a new request process.
		string sessionId = "session1";
		index = agntProcessorPtr->addRequest( sessionId, fields, ptrAct1, ptrAct1->getStart(), ptrAct1->getStop() );
				
		agntProcessorPtr->executeRequest(index, evnt.get());
		
		// More that one previous event could be in the evntScheduler.
		Event * evt = evnt.get()->getNextEvent();
		AddGeneratedBiddingObjectsEvent *etmp;
		bool eventFound = false;
		while  (evt != NULL){
			etmp = dynamic_cast<AddGeneratedBiddingObjectsEvent *>(evt);
			if (etmp == 0){
				evt = evnt.get()->getNextEvent();
			} else{
				eventFound = true;
				break;
			}
		}
		
		if (eventFound){
			int index2 = etmp->getIndex();		
			CPPUNIT_ASSERT( index == index2 );
		
			auctioningObjectDB_t *new_bids = etmp->getBiddingObjects();
			CPPUNIT_ASSERT( new_bids->size() == 1 );
			
			// Delete the bidding object created. 
			 auctioningObjectDBIter_t iter;
		    for (iter = new_bids->begin(); iter != new_bids->end(); iter++) {
				if (*iter != NULL) {
					// delete rule
					delete *iter;
				}
			}
        } 
		else {
			throw Error("Event was not generated");
		}
					
	} catch (Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
	
}
