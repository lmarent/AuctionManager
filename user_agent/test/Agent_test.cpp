/*
 * Test the Auctioner class.
 *
 * $Id: Auctioner.cpp 2015-08-12 14:29:00 amarentes $
 * $HeadURL: https://./test/Auctioner_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "EventAgent.h"
#include "Agent.h"
#include "anslp_ipap_xml_message.h"

class Agent_Test;

using namespace auction;

/*
 * We use a subclass for testing and make the test case a friend. This
 * way the test cases have access to protected methods and they don't have
 * to be public in mnslp_ipfix_fields.
 */
class agent_test : public Agent {
  public:
	agent_test( int _argc, char *_argv[] )
		: Agent( _argc, _argv)  { }

	friend class Agent_Test;
};


class Agent_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( Agent_Test );

    CPPUNIT_TEST( test );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void test();
	

  private:
    
	agent_test *agentPtr;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( Agent_Test );


void Agent_Test::setUp() 
{


	string commandLine = "netagent -c /usr/local/etc/auctionmanager/netagnt.conf.xml";
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
        // start up the netagent (this blocks until Ctrl-C !)
        agentPtr = new agent_test(argc, argv);
        
    } catch (Error &e) {
        cout << "Ter1minating netAgent on error: " << e.getError() << endl;
        if (agentPtr != NULL)
			saveDelete(agentPtr);
        throw e;
    }
				
}

void Agent_Test::tearDown() 
{
	
	if (agentPtr != NULL)
		saveDelete(agentPtr);

}

void Agent_Test::test() 
{

	try
	{
		
		cout << "Start the agent test " << endl;
        // going into main loop
        if (agentPtr != NULL){
			Event * evt = agentPtr->evnt.get()->getNextEvent();
			
			// Verifies that a new add resource request message event was generated
			cout << "Event:" << eventNames[evt->getType()] << endl;
			
			AddResourceRequestsEvent *arre = dynamic_cast<AddResourceRequestsEvent *>(evt);
			CPPUNIT_ASSERT( arre != NULL );
			
			// Process the add resource request message
			agentPtr->handleEvent(evt, NULL);
			
			// Verifies that an activate resource request interval events were created.
			// Another was executed inmediately.
			
			evt = agentPtr->evnt.get()->getNextEvent();
			ActivateResourceRequestIntervalEvent *arri = dynamic_cast<ActivateResourceRequestIntervalEvent *>(evt);
			CPPUNIT_ASSERT( arri != NULL );
			
			// Verifies that the activation produce the expected results.
			CPPUNIT_ASSERT(  agentPtr->asmp->getNumSessions() == 1 );
			
			sessionDB_t sessions = agentPtr->asmp->getSessions();
			sessionDBIter_t sesIter;
			AgentSession *asession = NULL;
			Session *ses = NULL;
			for (sesIter = sessions.begin(); sesIter != sessions.end(); ++sesIter){
				ses = *sesIter;
				asession = dynamic_cast<AgentSession *>(ses);
				CPPUNIT_ASSERT( asession->getResourceRequestSet() == "set1");
				CPPUNIT_ASSERT( asession->getResourceRequestName() == "1");
			}
			
			sessions.clear();
			
			string sessionId = ses->getSessionId();
			uint32_t seqNo = ses->getNextMessageId();
			
			// Verify that a pending message was created.
			pendingMessageListIter_t mesIter;
			int nbrMessages = 0;
			for ( mesIter = asession->beginMessages(); mesIter != asession->endMessages(); ++mesIter){
				++nbrMessages;
			}
			CPPUNIT_ASSERT( nbrMessages == 1);
			
			// Confirm manually the message reading an constant response and changing the message number.
			std::ifstream t("../../etc/ResponseRequestMessage.xml");
			std::string xmlMessage;
			
			t.seekg(0, std::ios::end);   
			xmlMessage.reserve(t.tellg());
			t.seekg(0, std::ios::beg);
			xmlMessage.assign((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());
			
			cout << "xml Message:" << xmlMessage << endl;
			
			anslp::msg::anslp_ipap_xml_message messRes;
			anslp::msg::anslp_ipap_message *ipapMesRes = messRes.from_message(xmlMessage);
			(ipapMesRes->ip_message).set_ackseqno(seqNo);
			
			agentPtr->evnt->addEvent( new ResponseCreateSessionEvent(sessionId, ipapMesRes->ip_message));

			evt = agentPtr->evnt.get()->getNextEvent();
			ResponseCreateSessionEvent *rcs = dynamic_cast<ResponseCreateSessionEvent *>(evt);
			CPPUNIT_ASSERT( rcs != NULL );

			// Process the add Response create session event.
			agentPtr->handleEvent(evt, NULL);
			
			// Verify that templates have been created.
			agentTemplateListIter_t tmplIter = agentPtr->agentTemplates.find(5);
		
			CPPUNIT_ASSERT( tmplIter != agentPtr->agentTemplates.end() );
						
			CPPUNIT_ASSERT( (tmplIter->second)->get_num_templates() == 8 );
			
			Session * ses2 = agentPtr->asmp->getSession(sessionId);
			CPPUNIT_ASSERT( ses2->beginMessages() == ses2->endMessages() );
			
			// Verify that a new auction was created.
			CPPUNIT_ASSERT( agentPtr->aucm->getNumAuctions() == 1);
			
			// Verify that a new request process was create
			requestProcessListIter_t reqIter;
			int nbrRequest = 0;
			for (reqIter = agentPtr->proc->begin(); reqIter != agentPtr->proc->end(); ++reqIter){
				nbrRequest++;
			}
			
			CPPUNIT_ASSERT( nbrRequest == 1 );
			
			// The next event is an activate auctions event, so if ok we proceed to handle it.
			evt = agentPtr->evnt.get()->getNextEvent();
			ActivateAuctionsEvent *aue = dynamic_cast<ActivateAuctionsEvent *>(evt);
			CPPUNIT_ASSERT( aue != NULL );
			
			// Handle the activate auctions event.
			agentPtr->handleEvent(evt, NULL);
			
			// Verify message confirmation.
			sessions = agentPtr->asmp->getSessions();
			for (sesIter = sessions.begin(); sesIter != sessions.end(); ++sesIter){
				ses = *sesIter;
				asession = dynamic_cast<AgentSession *>(ses);
				CPPUNIT_ASSERT( asession->getResourceRequestSet() == "set1");
				CPPUNIT_ASSERT( asession->getResourceRequestName() == "1");
			}

			nbrMessages = 0;
			for ( mesIter = asession->beginMessages(); mesIter != asession->endMessages(); ++mesIter){
				++nbrMessages;
			}
			
			CPPUNIT_ASSERT( nbrMessages == 0);
						
			// Verify that a new Push execute event was created.
			evt = agentPtr->evnt.get()->getNextEvent();
			PushExecutionEvent *pee = dynamic_cast<PushExecutionEvent *>(evt);
			CPPUNIT_ASSERT( pee != NULL );
			
			// Handle the activate auctions event.
			agentPtr->handleEvent(evt, NULL);
			
			// Verify that a new Add-Generated-Bidding-Objects event was created.
			evt = agentPtr->evnt.get()->getNextEvent();
			AddGeneratedBiddingObjectsEvent *agboe = dynamic_cast<AddGeneratedBiddingObjectsEvent *>(evt);
			CPPUNIT_ASSERT( agboe != NULL );
			
			biddingObjectDB_t *new_bids = ((AddGeneratedBiddingObjectsEvent *)evt)->getBiddingObjects();
			
			biddingObjectDBIter_t bidIter;
			for (bidIter = new_bids->begin(); bidIter != new_bids->end(); ++bidIter){
				cout << (*bidIter)->getInfo() << endl;
			} 
			
			// Handle the Add-Generated-Bidding-Objects event.
			agentPtr->handleEvent(evt, NULL);
			
			CPPUNIT_ASSERT( agentPtr->bidm->getNumBiddingObjects() == 1);
			
			while (evt != NULL ){
				cout << "Event:" << eventNames[evt->getType()] << endl;			
				evt = agentPtr->evnt.get()->getNextEvent();
			}	
			
		}	

		
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}


}
