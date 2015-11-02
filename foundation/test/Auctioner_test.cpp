/*
 * Test the Auctioner class.
 *
 * $Id: Auctioner.cpp 2015-08-12 14:29:00 amarentes $
 * $HeadURL: https://./test/Auctioner_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Auctioner.h"

class Auctioner_Test;

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
		
	try
	{
        // start up the netmate (this blocks until Ctrl-C !)
        auctionerPtr = new auctioner_test(argc, argv);
        
        string filename = DEF_SYSCONFDIR "/example_bids1.xml";
		auctionerPtr->evnt->addEvent(new AddBidsEvent(filename));

    } catch (Error &e) {
        cout << "Terminating Auctioner on error: " << e.getError() << endl;
    }
				
}

void Auctioner_Test::tearDown() 
{
	
	saveDelete(auctionerPtr);

}

void Auctioner_Test::test() 
{

	try
	{

        // going into main loop
        auctionerPtr->run();

		
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}


}
