/*
 * Test the Field_Value class.
 *
 * $Id: mnslp_fields.cpp 2014-11-28 10:16:00 amarentes $
 * $HeadURL: https://./test/mnslp_fields_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Timeval.h"
#include <unistd.h>


class Timeval_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( Timeval_Test );

	CPPUNIT_TEST( test );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void test();

  private:
        
};

CPPUNIT_TEST_SUITE_REGISTRATION( Timeval_Test );


void Timeval_Test::setUp() 
{
}

void Timeval_Test::tearDown() 
{
}

void Timeval_Test::test() 
{

	struct timeval time1;
	struct timeval time2;
	std::string time1Str, time2Str;
	
	Timeval::gettimeofday(&time1,NULL);
	sleep(2);
	Timeval::gettimeofday(&time2,NULL);
	time1Str = Timeval::toString(time1.tv_sec);
	time2Str = Timeval::toString(time2.tv_sec);
	
	std::cout << "time1:" << time1Str << std::endl;
	std::cout << "time2:" << time2Str << std::endl;

}
