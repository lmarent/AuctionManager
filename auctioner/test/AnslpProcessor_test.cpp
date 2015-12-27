/*
 * Test the AnslpProcessor class.
 *
 * $Id: AnslpProcessor_test.cpp 2015-12-25 8:58:00 amarentes $
 * $HeadURL: https://./test/AnslpProcessor_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "AnslpProcessor.h"
#include "Constants.h"
#include "ConstantsAum.h"
#include "ConfigManager.h"
#include "EventScheduler.h"
#include "AuctionManager.h"

using namespace auction;

class AnslpProcessor_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( AnslpProcessor_Test );

	CPPUNIT_TEST( testCheckEvent );
	CPPUNIT_TEST( testAddSessionEvent );
	CPPUNIT_TEST( testResponseCheckSessionEvent );
	CPPUNIT_TEST( testResponseAddSessionEvent );
	CPPUNIT_TEST( testAuctionInteractionEvent );

	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void testCheckEvent();
	void testAddSessionEvent();
	void testResponseCheckSessionEvent();
	void testResponseAddSessionEvent();
	void testAuctionInteractionEvent();
	void test();

  private:

	  ConfigManager *configManagerPtr;
	  AnslpProcessor *anslpProcessorPtr;
      anslp::FastQueue *inputQueue;
	  
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( AnslpProcessor_Test );


void AnslpProcessor_Test::setUp() 
{
	
	try
	{
		string commandLine = "auctioner";
		char *cstr = new char[commandLine.length() + 1];
		strcpy(cstr, commandLine.c_str());
		
		enum { kMaxArgs = 64 };
		int argc = 0;
		char *argv[kMaxArgs];

		char *p2 = strtok(cstr, " ");
		while (p2 && argc < kMaxArgs-1)
		{
			argv[argc++] = p2;
			p2 = strtok(0, " ");
		}
		argv[argc] = 0;
					
		const string configDTD = DEF_SYSCONFDIR "/netaum.conf.dtd";
		const string configFileName = NETAUM_DEFAULT_CONFIG_FILE;
		configManagerPtr = new ConfigManager(configDTD, configFileName, argv[0]);
				
		anslpProcessorPtr = new AnslpProcessor( configManagerPtr , 0);
				        
	}
	catch(Error &e){
		cout << "Error:" << e.getError() << endl << flush;
		throw e;
	}
	
}

void AnslpProcessor_Test::tearDown() 
{
	
	// This deletes the config Manager ptr
	delete(anslpProcessorPtr); 
			
}

void AnslpProcessor_Test::testCheckEvent()
{

	try
	{
		eventVec_t eventVec;

		anslp::FastQueue retQueue;
		
		// Creates a check event.
		anslp::CheckEvent *checkEvt = new anslp::CheckEvent(&retQueue);
		
		anslpProcessorPtr->get_fqueue()->enqueue(checkEvt);
		
		anslpProcessorPtr->handleFDEvent(&eventVec, NULL, NULL, NULL);
		
		CPPUNIT_ASSERT( eventVec.size() == 1 );
		

	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}	
}

void AnslpProcessor_Test::testAddSessionEvent() 
{

	try
	{
		eventVec_t eventVec;
		
		anslp::FastQueue retQueue;
		
		// Creates a check event.
		anslp::AddSessionEvent *evt = new anslp::AddSessionEvent(&retQueue);
		
		anslpProcessorPtr->get_fqueue()->enqueue(evt);
		
		anslpProcessorPtr->handleFDEvent(&eventVec, NULL, NULL, NULL);
		
		CPPUNIT_ASSERT( eventVec.size() == 1 );

	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}	
}

void AnslpProcessor_Test::testResponseCheckSessionEvent() 
{

	try
	{
		eventVec_t eventVec;
		
		// Creates a check event.
		anslp::ResponseCheckSessionEvent *evt = new anslp::ResponseCheckSessionEvent();
		
		anslpProcessorPtr->get_fqueue()->enqueue(evt);
		
		anslpProcessorPtr->handleFDEvent(&eventVec, NULL, NULL, NULL);
		
		CPPUNIT_ASSERT( eventVec.size() == 0 );


	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}	
}

void AnslpProcessor_Test::testResponseAddSessionEvent()
{

	try
	{
		eventVec_t eventVec;
		
		// Creates a check event.
		anslp::ResponseAddSessionEvent *evt = new anslp::ResponseAddSessionEvent();
		
		anslpProcessorPtr->get_fqueue()->enqueue(evt);
		
		anslpProcessorPtr->handleFDEvent(&eventVec, NULL, NULL, NULL);
		
		CPPUNIT_ASSERT( eventVec.size() == 0 );


	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}	
}


void AnslpProcessor_Test::testAuctionInteractionEvent() 
{

	try
	{
		eventVec_t eventVec;
		
		// Creates an auction interaction event.
		anslp::AuctionInteractionEvent *evt = new anslp::AuctionInteractionEvent();
		
		anslpProcessorPtr->get_fqueue()->enqueue(evt);
		
		anslpProcessorPtr->handleFDEvent(&eventVec, NULL, NULL, NULL);
		
		CPPUNIT_ASSERT( eventVec.size() == 1 );

	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}	
}

