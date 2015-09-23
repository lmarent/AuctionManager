/*
 * Test the FieldDefParser class.
 *
 * $Id: FieldDefParser.cpp 2015-07-24 15:50:00 amarentes $
 * $HeadURL: https://./test/FieldDefParser_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "FieldDefParser.h"

using namespace auction;

class FieldDefParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( FieldDefParser_Test );

	CPPUNIT_TEST( testParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testParser();

  private:
    
    FieldDefParser *ptrFieldParsers;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( FieldDefParser_Test );


void FieldDefParser_Test::setUp() 
{
	const string filename = DEF_SYSCONFDIR "/fielddef.xml";

	try
	{
		
		ptrFieldParsers = new FieldDefParser(filename);
		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void FieldDefParser_Test::tearDown() 
{
	delete(ptrFieldParsers);
}

void FieldDefParser_Test::testParser() 
{
	fieldDefList_t *list = new fieldDefList_t();
	
	ptrFieldParsers->parse(list);
		
	CPPUNIT_ASSERT( list->size() == 7 );
	
	delete(list);
}


