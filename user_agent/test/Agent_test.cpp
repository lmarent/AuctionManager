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
	
	void checkRemoveAuctionEvent(Event *evt);
	void checkRemoveRequestEvent(Event *evt);
	void checkRemoveBiddingObjectEvent(Event *evt);
	

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
        cout << "Terminating netAgent on error: " << e.getError() << endl;
        if (agentPtr != NULL)
			saveDelete(agentPtr);
        throw e;
    }
				
}

void Agent_Test::tearDown() 
{
	cout << "Teardown agent test " << endl;
	
	if (agentPtr != NULL)
		saveDelete(agentPtr);

}

void Agent_Test::test() 
{

	try
	{
		
        // going into main loop
        if (agentPtr != NULL){
	
			// test the interval intersect function
			time_t time1, time2, time3, time4, time5, time6;
			time_t start, stop;
			
			time1 = time(NULL);
			time2 = time1 + 300;
			
			time3 = time1 - 300;
			time4 = time1 - 200;
			
			agentPtr->intersectInterval(time1, time2, time3, time4, start, stop);
			CPPUNIT_ASSERT( start == time3 );
			CPPUNIT_ASSERT( stop == time4 );

			time3 = time1 - 300;
			time4 = time1 + 100;
			agentPtr->intersectInterval(time1, time2, time3, time4, start, stop);
			CPPUNIT_ASSERT( start == time1 );
			CPPUNIT_ASSERT( stop == time4 );

			time3 = time1 + 100;
			time4 = time1 + 200;
			agentPtr->intersectInterval(time1, time2, time3, time4, start, stop);
			CPPUNIT_ASSERT( start == time3 );
			CPPUNIT_ASSERT( stop == time4 );

			time3 = time1 + 200;
			time4 = time1 + 400;
			agentPtr->intersectInterval(time1, time2, time3, time4, start, stop);
			CPPUNIT_ASSERT( start == time3 );
			CPPUNIT_ASSERT( stop == time2 );

			time3 = time1 + 350;
			time4 = time1 + 400;
			agentPtr->intersectInterval(time1, time2, time3, time4, start, stop);
			CPPUNIT_ASSERT( start == time3 );
			CPPUNIT_ASSERT( stop == time4 );
			
			
			Event * evt = agentPtr->evnt.get()->getNextEvent();
			
			// Verifies that a new add resource request message event was generated
			cout << "Event:" << eventNames[evt->getType()] << endl;
			
			AddResourceRequestsEvent *arre = dynamic_cast<AddResourceRequestsEvent *>(evt);
			CPPUNIT_ASSERT( arre != NULL );
			
			cout << "nbr queue:" << agentPtr->anslpc->getAnslpDeamon()->getInstallQueue()->size() << endl;
			
			// Process the add resource request message
			agentPtr->handleEvent(evt, NULL);
						
			// Verifies that an activate resource request interval events were created.
			// Another was executed inmediately. Take into account that an activate
			// resource request event is lost, because we skip its execution to ease the cases.
			
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
			
			cout << "session id created:" << sessionId << endl; 
						
			uint32_t seqNo = ses->getNextMessageId();
			
			// Verify that a pending message was created.
			pendingMessageListIter_t mesIter;
			int nbrMessages = 0;
			for ( mesIter = asession->beginMessages(); mesIter != asession->endMessages(); ++mesIter){
				++nbrMessages;
			}
			CPPUNIT_ASSERT( nbrMessages == 1);
			
			// Confirm manually the message reading a constant response and changing the message number.
			std::ifstream t("../../etc/ResponseRequestMessage2.xml");
			std::string xmlMessage;
			
			t.seekg(0, std::ios::end);   
			xmlMessage.reserve(t.tellg());
			t.seekg(0, std::ios::beg);
			xmlMessage.assign((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());
			
			//cout << "xml Message:" << xmlMessage << endl;
			
			anslp::msg::anslp_ipap_xml_message messRes;
			anslp::msg::anslp_ipap_message *ipapMesRes = messRes.from_message(xmlMessage);
			(ipapMesRes->ip_message).set_ackseqno(seqNo);
			std::string xmlRespMes = messRes.get_message(*ipapMesRes);
			
			//cout << "xmlResponse  Message:" << xmlRespMes << endl;
			
			// Now we create the message to configure the session.
									
			anslp::AnslpEvent *evtAddRet = agentPtr->anslpc->getAnslpDeamon()->getInstallQueue()->dequeue_timedwait(1000);

			anslp::AddAnslpSessionEvent *aase = 
					dynamic_cast<anslp::AddAnslpSessionEvent *>(evtAddRet);			
			
			auction::ConfigureSessionEvent *evConf = 
				new auction::ConfigureSessionEvent(aase->getSession(), aase->getAnslpSession());
			
			string anslpsession = aase->getAnslpSession();
			
			cout << "anslpsession:" << anslpsession << endl;
			
			agentPtr->evnt->addEvent( evConf );
			
			evt = agentPtr->evnt.get()->getNextEvent();
			ConfigureSessionEvent *evConf2 = dynamic_cast<ConfigureSessionEvent *>(evt);
			CPPUNIT_ASSERT( evConf2 != NULL );
			
			// Handle the activate session event.
			agentPtr->handleEvent(evt, NULL);			
			
			// Verify the active session handler.
			sessions = agentPtr->asmp->getSessions();
			for (sesIter = sessions.begin(); sesIter != sessions.end(); ++sesIter){
				ses = *sesIter;
				asession = dynamic_cast<AgentSession *>(ses);
				CPPUNIT_ASSERT( asession->getResourceRequestSet() == "set1");
				CPPUNIT_ASSERT( asession->getResourceRequestName() == "1");
				CPPUNIT_ASSERT( asession->getState() == SS_ACTIVE );
				CPPUNIT_ASSERT( asession->getAnlspSession() == anslpsession );
			}
			
			sessions.clear();
						
			// Get the response from the anslp application.
			
			anslp::mspec_rule_key key;
			anslp::FastQueue retQueue;
			
			CreateSessionEvent *rcs3 = new CreateSessionEvent(anslpsession, &retQueue);
			rcs3->setObject(key, ipapMesRes);
			
			agentPtr->evnt->addEvent( rcs3 );

			evt = agentPtr->evnt.get()->getNextEvent();
			CreateSessionEvent *rcs = dynamic_cast<CreateSessionEvent *>(evt);
			CPPUNIT_ASSERT( rcs != NULL );
			
			// Process the add Response create session event.
			agentPtr->handleEvent(evt, NULL);
								
			// Verify that templates have been created.
			agentTemplateListIter_t tmplIter = agentPtr->agentTemplates.find(1);
		
			CPPUNIT_ASSERT( tmplIter != agentPtr->agentTemplates.end() );
						
			CPPUNIT_ASSERT( (tmplIter->second)->get_num_templates() == 16 );
			
			Session * ses2 = agentPtr->asmp->getSession(sessionId);
			CPPUNIT_ASSERT( ses2->beginMessages() == ses2->endMessages() );
			
			// Verify that a new auction was created.
			CPPUNIT_ASSERT( agentPtr->aucm->getNumAuctioningObjects() == 2);

			// Print both auctions.
			
			cout << *(agentPtr->aucm) << endl;
						
			// Verify that a new request process was created
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

			for (reqIter = agentPtr->proc->begin(); reqIter != agentPtr->proc->end(); ++reqIter){
				nbrRequest++;
			}

			CPPUNIT_ASSERT( nbrRequest == 2 );
			
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
			
			auctioningObjectDB_t *new_bids = ((AddGeneratedBiddingObjectsEvent *)evt)->getBiddingObjects();
			
			int numBids = 0;
			auctioningObjectDBIter_t bidIter;
			for (bidIter = new_bids->begin(); bidIter != new_bids->end(); ++bidIter){
				numBids++;
			} 
						
			CPPUNIT_ASSERT( numBids == 2);
			
			// Handle the Add-Generated-Bidding-Objects event.
			agentPtr->handleEvent(evt, NULL);
						
			CPPUNIT_ASSERT( agentPtr->bidm->getNumAuctioningObjects() == 2);
						
			// Verify that a new activate bidding Object event was created.
			evt = agentPtr->evnt.get()->getNextEvent();
			
			cout << *(agentPtr->evnt.get()) << endl;
			
			int numEvents = 0;
			while (numEvents <=1){
				
				// Depending on execution time, it can create first the activate event or the trasmit event
				if (evt->getType() == TRANSMIT_BIDDING_OBJECTS) {
										
					// Verify that a new transmit bidding Object event was created.
					TransmitBiddingObjectsEvent *tboe = dynamic_cast<TransmitBiddingObjectsEvent *>(evt);
					CPPUNIT_ASSERT( tboe != NULL );
											
					numBids = 0;
					new_bids = ((TransmitBiddingObjectsEvent *)evt)->getBiddingObjects();
					for (bidIter = new_bids->begin(); bidIter != new_bids->end(); ++bidIter){
						numBids++;
					} 
											
					CPPUNIT_ASSERT( numBids == 2);

					// Handle the Transmit-Bidding-Objects event.
					agentPtr->handleEvent(evt, NULL);
					
				}	   
				else if (evt->getType() == ACTIVATE_BIDDING_OBJECTS) {
										
					ActivateBiddingObjectsEvent *aboe = dynamic_cast<ActivateBiddingObjectsEvent *>(evt);
					CPPUNIT_ASSERT( aboe != NULL );
						
					numBids = 0;
					new_bids = ((ActivateBiddingObjectsEvent *)evt)->getBiddingObjects();
					for (bidIter = new_bids->begin(); bidIter != new_bids->end(); ++bidIter){
						numBids++;
					} 
						
					CPPUNIT_ASSERT( numBids == 2);
						
					// Handle the Activate-Bidding-Objects event.
					agentPtr->handleEvent(evt, NULL);
				}
				else {	
				   // We are expecting a transmit or activate bidding object 
				   CPPUNIT_ASSERT( 1 == 0 );	
				}
				numEvents++;
				evt = agentPtr->evnt.get()->getNextEvent();
			}
						
			numEvents = 0;
			while (numEvents <=2){
							
				if (evt->getType() == REMOVE_RESOURCE_REQUEST_INTERVAL) {
					checkRemoveRequestEvent(evt);
				}		
				else if (evt->getType() == REMOVE_AUCTIONS) {
					checkRemoveAuctionEvent(evt);
				}	
				else if (evt->getType() == REMOVE_BIDDING_OBJECTS) {
					checkRemoveBiddingObjectEvent(evt);
				}	
				numEvents++;
				evt = agentPtr->evnt.get()->getNextEvent();
			}
						
		}	
		
		
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}
}

void Agent_Test::checkRemoveAuctionEvent(Event *evt)
{

	RemoveAuctionsEvent *rae = dynamic_cast<RemoveAuctionsEvent *>(evt);
	CPPUNIT_ASSERT( rae != NULL );
			
	struct timeval before = rae->getTime();
	agentPtr->evnt.get()->addEvent(rae);
			
	time_t now = time(NULL);
	now = now + 1;

	// Verify that auction's delete reschedule work.
	if (rae->isIncluded(0)){
		agentPtr->evnt.get()->rescheduleAuctionDelete(0, now);
	}
						
	struct timeval after = rae->getTime();
	CPPUNIT_ASSERT( before.tv_sec + 1 == after.tv_sec );

	evt = agentPtr->evnt.get()->getNextEvent();
	rae = dynamic_cast<RemoveAuctionsEvent *>(evt);
	CPPUNIT_ASSERT( rae != NULL );
						
	auctioningObjectDB_t *auctions = rae->getAuctions();
	auctioningObjectDBIter_t aucIter;
	int numAuct = 0;
	for (aucIter = auctions->begin(); aucIter != auctions->end(); ++aucIter){
		numAuct++;
	} 
			
	CPPUNIT_ASSERT( numAuct == 2 );
		
	// Handle the Remove-Auctions event.
	agentPtr->handleEvent(evt, NULL);
			
	// Verify the auction depletion.
	CPPUNIT_ASSERT( agentPtr->aucm->getNumAuctioningObjects() == 0);

}

void Agent_Test::checkRemoveRequestEvent(Event *evt)
{
	RemoveResourceRequestIntervalEvent *rrrie = dynamic_cast<RemoveResourceRequestIntervalEvent *>(evt);
	CPPUNIT_ASSERT( rrrie != NULL );
			
	ResourceRequest * request = rrrie->getResourceRequest();
			
	CPPUNIT_ASSERT( request != NULL );
			
	// Handle the Remove-Request-Interval event.
	agentPtr->handleEvent(evt, NULL);

	// Verify that a new request process was create
	int nbrRequest = 0;
	requestProcessListIter_t reqIter;
	for (reqIter = agentPtr->proc->begin(); reqIter != agentPtr->proc->end(); ++reqIter){
		nbrRequest++;
	}
			
	CPPUNIT_ASSERT( nbrRequest == 0 );

}

void Agent_Test::checkRemoveBiddingObjectEvent(Event *evt)
{
	RemoveBiddingObjectsEvent *rboe = dynamic_cast<RemoveBiddingObjectsEvent *>(evt);
	CPPUNIT_ASSERT( rboe != NULL );

	auctioningObjectDB_t *auctions = rboe->getBiddingObjects();
	auctioningObjectDBIter_t bidIter;
	int numBids = 0;
	for (bidIter = auctions->begin(); bidIter != auctions->end(); ++bidIter){
		numBids++;
	} 
				
	CPPUNIT_ASSERT( numBids == 1 );

			
	// Handle the Remove-Bidding Objects event.
	agentPtr->handleEvent(evt, NULL);
			
	CPPUNIT_ASSERT( agentPtr->bidm->getNumAuctioningObjects() == 1);
}
