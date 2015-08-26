/*
 * Test the FieldValParser class.
 *
 * $Id: FieldValParser.cpp 2015-07-24 15:50:00 amarentes $
 * $HeadURL: https://./test/FieldValParser_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "FieldValParser.h"


class FieldValParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( FieldValParser_Test );
	
	CPPUNIT_TEST( testParser );
	
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void testParser();

  private:
    
    FieldValParser *ptrFieldValParser;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( FieldValParser_Test );


void FieldValParser_Test::setUp() 
{
	const string filename = DEF_SYSCONFDIR "/fieldval.xml";
	try
	{
		
		ptrFieldValParser = new FieldValParser(filename);
	
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
	
}

void FieldValParser_Test::tearDown() 
{
	delete(ptrFieldValParser);
}


void FieldValParser_Test::testParser() 
{

	fieldValList_t *list = new fieldValList_t();
		
	ptrFieldValParser->parse(list);
		
	CPPUNIT_ASSERT( list->size() == 1 );
	
	delete(list);
}


