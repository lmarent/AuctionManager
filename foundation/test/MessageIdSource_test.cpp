/*
 * Test the MessageIdSource_test class.
 *
 * $Id: MessageIdSource_Test.cpp 2015-08-04 14:56:00 amarentes $
 * $HeadURL: https://./test/MessageIdSource_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MessageIdSource.h"


using namespace auction;

class MessageIdSource_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( MessageIdSource_Test );

	CPPUNIT_TEST( testId );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void testId();

  private:
    
    MessageIdSource *ptrIdSource;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( MessageIdSource_Test );


void MessageIdSource_Test::setUp() 
{

	ptrIdSource = new MessageIdSource();
	
	

}

void MessageIdSource_Test::tearDown() 
{
	delete(ptrIdSource);
		
}

void MessageIdSource_Test::testId() 
{
	// Non Unique test
	uint32_t id1 = ptrIdSource->newId();
	uint32_t id2 = ptrIdSource->newId();
	
	CPPUNIT_ASSERT( id1 != id2 );
	
	CPPUNIT_ASSERT( (id1 + 1) == id2 );
	
	ptrIdSource->setNewId(UINT32_MAX-1);
	
	uint32_t id4 = ptrIdSource->newId();
	
	CPPUNIT_ASSERT( id4  == 0 );

	ptrIdSource->setNewId( 2147483648 );
	
	uint32_t id5 = ptrIdSource->newId();
	
	CPPUNIT_ASSERT( id5  == 2147483649 );	
}

