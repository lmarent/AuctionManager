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
			
			// Test the arrival of a new session.
			
			string sessionId = "490a2d28-8649-11e5-a106-080027b0f309";
			std::ifstream t("../../etc/sessionRequest.xml");
			std::string xmlMessage;
			
			t.seekg(0, std::ios::end);   
			xmlMessage.reserve(t.tellg());
			t.seekg(0, std::ios::beg);

			xmlMessage.assign((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());

			// check session creation.
			anslp::msg::anslp_ipap_xml_message mess;
			anslp::msg::anslp_ipap_message *ipap_mes = mess.from_message(xmlMessage);
			auctionerPtr->evnt->addEvent( new CreateCheckSessionEvent(sessionId, ipap_mes->ip_message));

			evt = auctionerPtr->evnt.get()->getNextEvent();
			CreateCheckSessionEvent *ccse = dynamic_cast<CreateCheckSessionEvent *>(evt);
			CPPUNIT_ASSERT( ccse != NULL );
			
			auctionerPtr->handleEvent(evt, NULL);
			
			// once check the message, we proceed to create the session,
			auctionerPtr->evnt->addEvent( new CreateSessionEvent(sessionId, ipap_mes->ip_message));

			evt = auctionerPtr->evnt.get()->getNextEvent();
			CreateSessionEvent *cse = dynamic_cast<CreateSessionEvent *>(evt);
			CPPUNIT_ASSERT( cse != NULL );
							
			auctionerPtr->handleEvent(evt, NULL);
			
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

			saveDelete(ipap_mes);
			
		 }
		
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}


}
