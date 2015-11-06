/*
 * Test the Auctioner class.
 *
 * $Id: Auctioner.cpp 2015-08-12 14:29:00 amarentes $
 * $HeadURL: https://./test/Auctioner_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "Agent.h"

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
        
        //string filename = DEF_SYSCONFDIR "/example_bids1.xml";
		//auctionerPtr->evnt->addEvent(new AddBidsEvent(filename));

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

        // going into main loop
        if (agentPtr != NULL)
			agentPtr->run();

		
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}


}
