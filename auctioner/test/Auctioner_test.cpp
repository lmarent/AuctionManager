/*
 * Test the Auctioner class.
 *
 * $Id: Auctioner.cpp 2015-08-12 14:29:00 amarentes $
 * $HeadURL: https://./test/Auctioner_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "EventAuctioner.h"
#include "Auctioner.h"
#include <string>
#include <fstream>
#include <streambuf>

class Auctioner_Test;


using namespace auction;

/*
 * We use a subclass for testing and make the test case a friend. This
 * way the test cases have access to protected methods and they don't have
 * to be public in mnslp_ipfix_fields.
 */
class auctioner_test : public Auctioner {
  public:
	auctioner_test( int _argc, char *_argv[] )
		: Auctioner( _argc, _argv)  { }

	friend class Auctioner_Test;
};


class Auctioner_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( Auctioner_Test );

    CPPUNIT_TEST( test );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void test();
	

  private:
    
	auctioner_test *auctionerPtr;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( Auctioner_Test );


void Auctioner_Test::setUp() 
{


	string commandLine = "auctionManager -c " DEF_SYSCONFDIR "/netaum.conf.xml";
	
	cout << "commandLine:" << commandLine << endl;
	
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
		
	try
	{
        // start up the netmate (this blocks until Ctrl-C !)
        auctionerPtr = new auctioner_test(argc, argv);
        
    } catch (Error &e) {
        cout << "Terminating Auctioner on error: " << e.getError() << endl;
    }
				
}

void Auctioner_Test::tearDown() 
{
	
	if (auctionerPtr != NULL)
		saveDelete(auctionerPtr);

}

void Auctioner_Test::test() 
{

	try
	{

        // going into main loop
         if (auctionerPtr != NULL){
			Event * evt = auctionerPtr->evnt.get()->getNextEvent();
			
			// Verifies that a new add auctions event was generated			
			AddAuctionsEvent *aae = dynamic_cast<AddAuctionsEvent *>(evt);
			CPPUNIT_ASSERT( aae != NULL );
			
			// Process the add auction event.
			auctionerPtr->handleEvent(evt, NULL);
			
			// Verifies the number of auctions
			CPPUNIT_ASSERT( auctionerPtr->aucm->getNumAuctions() == 1);
			
			// Verifies that a new PushExecutionEvent was created
			evt = auctionerPtr->evnt.get()->getNextEvent();
			PushExecutionEvent *pee = dynamic_cast<PushExecutionEvent *>(evt);
			CPPUNIT_ASSERT( pee != NULL );
			
			// Process the push execution event.
			auctionerPtr->handleEvent(evt, NULL);
			
			
			// Test the arrival of a new session.
			
			anslp::session_id *session1 = new anslp::session_id();
			string sessionId = session1->to_string();
			saveDelete(session1);
			
			std::ifstream t("../../etc/sessionRequest.xml");
			std::string xmlMessage;
			
			t.seekg(0, std::ios::end);   
			xmlMessage.reserve(t.tellg());
			t.seekg(0, std::ios::beg);
			
			xmlMessage.assign((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());
				
			// Replace the start and stop time for the session request
			time_t now = time(NULL);
			ostringstream a;
			a << now;
			string snow = a.str();
			xmlMessage.replace(1506, 10 , snow);
								
			time_t now2 = time(NULL) + 300;
			ostringstream a2;
			a2 << now2;
			string snow2 = a2.str();
			xmlMessage.replace(1568, 10, snow2);
			
			// check session creation.
			auctionerPtr->evnt->addEvent( new CreateCheckSessionEvent(sessionId, xmlMessage));

			evt = auctionerPtr->evnt.get()->getNextEvent();
			CreateCheckSessionEvent *ccse = dynamic_cast<CreateCheckSessionEvent *>(evt);
			CPPUNIT_ASSERT( ccse != NULL );
			
			auctionerPtr->handle_event_immediate_respond(evt, NULL);
			
			// once check the message, we proceed to create the session,
			auctionerPtr->evnt->addEvent( new CreateSessionEvent(sessionId, xmlMessage));

			evt = auctionerPtr->evnt.get()->getNextEvent();
			CreateSessionEvent *cse = dynamic_cast<CreateSessionEvent *>(evt);
			CPPUNIT_ASSERT( cse != NULL );
							
			auctionerPtr->handle_event_immediate_respond(evt, NULL);
			
			// TODO AM: what to do if there is no auction satisfying the given parameters.
			
			// Verify the session created
			CPPUNIT_ASSERT(  auctionerPtr->sesm->getNumSessions() == 1 );
			
			sessionDB_t sessions = auctionerPtr->sesm->getSessions();
			sessionDBIter_t sesIter;
			Session *ses = NULL;
			for (sesIter = sessions.begin(); sesIter != sessions.end(); ++sesIter){
				ses = *sesIter;
				CPPUNIT_ASSERT( ses->getSessionId() == sessionId);
			}

			// Verify that a pending message was created.
			pendingMessageListIter_t mesIter;
			int nbrMessages = 0;
			for ( mesIter = ses->beginMessages(); mesIter != ses->endMessages(); ++mesIter){
				++nbrMessages;
			}
			CPPUNIT_ASSERT( nbrMessages == 1);

			std::ifstream t2("../../etc/example_xml_biddingobject.xml");
			std::string xmlMessage2;
			
			t2.seekg(0, std::ios::end);   
			xmlMessage2.reserve(t2.tellg());
			t2.seekg(0, std::ios::beg);

			xmlMessage2.assign((std::istreambuf_iterator<char>(t2)),std::istreambuf_iterator<char>());

			// Replace the start and stop time for the bidding object
			time_t now3 = time(NULL);
			ostringstream a3;
			a3 << now3;
			string snow3 = a3.str();
			xmlMessage2.replace(2287, 10 , snow3);
								
			time_t now4 = time(NULL) + 300;
			ostringstream a4;
			a4 << now4;
			string snow4 = a4.str();
			xmlMessage2.replace(2349, 10, snow4);


			// Auction Interaction event.
			auctionerPtr->evnt->addEvent( new AuctionInteractionEvent(sessionId, xmlMessage2));

			evt = auctionerPtr->evnt.get()->getNextEvent();
			AuctionInteractionEvent *aie = dynamic_cast<AuctionInteractionEvent *>(evt);
			CPPUNIT_ASSERT( aie != NULL );
			
			auctionerPtr->handleEvent(evt, NULL);
			
			// Verify that a new bidding object was created.
			CPPUNIT_ASSERT( auctionerPtr->bidm->getNumBiddingObjects() == 1 );
			
			evt = auctionerPtr->evnt.get()->getNextEvent();
			
			// Verify that a new activate bidding object was generated.
			ActivateBiddingObjectsEvent *aboe = dynamic_cast<ActivateBiddingObjectsEvent *>(evt);
			CPPUNIT_ASSERT( aboe != NULL );
			
			auctionerPtr->handleEvent(evt, NULL);
			
			evt = auctionerPtr->evnt.get()->getNextEvent();
			
			// Verify that a new add bidding object to auction event was generated.
			InsertBiddingObjectsAuctionEvent *iboae = dynamic_cast<InsertBiddingObjectsAuctionEvent *>(evt);
			CPPUNIT_ASSERT( iboae != NULL );
			
			auctionerPtr->handleEvent(evt, NULL);
			
			// Verify that the bidding object was inserted into the auction process.
			auctionProcessListIter_t actProcessIter = auctionerPtr->proc->begin();
			CPPUNIT_ASSERT( actProcessIter->second.getBids()->size() == 1 );

			evt = auctionerPtr->evnt.get()->getNextEvent();
			
			// Verify that a push execution event was generated and execute it.
			pee = dynamic_cast<PushExecutionEvent *>(evt);
			CPPUNIT_ASSERT( pee != NULL );	
			auctionerPtr->handleEvent(evt, NULL);
	
			// Verify that a bidding generation event was created and execute it.
			evt = auctionerPtr->evnt.get()->getNextEvent();
			AddGeneratedBiddingObjectsEvent *agboe = dynamic_cast<AddGeneratedBiddingObjectsEvent *>(evt);
			CPPUNIT_ASSERT( agboe != NULL );
			
			biddingObjectDB_t *new_bids = ((AddGeneratedBiddingObjectsEvent *)evt)->getBiddingObjects();
			
			int numAllocs = 0;
			biddingObjectDBIter_t bidIter;
			for (bidIter = new_bids->begin(); bidIter != new_bids->end(); ++bidIter){
				numAllocs++;
			} 
			
			CPPUNIT_ASSERT( numAllocs == 1);
			auctionerPtr->handleEvent(evt, NULL);
			
			// Verify that it creates a new bidding object.
			// Verify that a new bidding object was created.
			CPPUNIT_ASSERT( auctionerPtr->bidm->getNumBiddingObjects() == 2 );

			evt = auctionerPtr->evnt.get()->getNextEvent();
			
			// Verify that a new activate bidding object was generated.
			aboe = dynamic_cast<ActivateBiddingObjectsEvent *>(evt);
			CPPUNIT_ASSERT( aboe != NULL );
			
			auctionerPtr->handleEvent(evt, NULL);
		
			evt = auctionerPtr->evnt.get()->getNextEvent();
			
			// Verify that a transmit bidding object was generated.
			TransmitBiddingObjectsEvent *tboe = dynamic_cast<TransmitBiddingObjectsEvent *>(evt);
			CPPUNIT_ASSERT( tboe != NULL );

			numAllocs = 0;
			new_bids = ((TransmitBiddingObjectsEvent *)evt)->getBiddingObjects();
			for (bidIter = new_bids->begin(); bidIter != new_bids->end(); ++bidIter){
				numAllocs++;
			} 
			
			CPPUNIT_ASSERT( numAllocs == 1);

			// Handle the Transmit-Bidding-Objects event.
			auctionerPtr->handleEvent(evt, NULL);
						
			evt = auctionerPtr->evnt.get()->getNextEvent();
			while ( evt != NULL ){
				cout << eventNames[evt->getType()].c_str() << "time:" << Timeval::toString(evt->getTime()) << endl;
				evt = auctionerPtr->evnt.get()->getNextEvent();
			}			
					
			
		 }
		
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}


}
