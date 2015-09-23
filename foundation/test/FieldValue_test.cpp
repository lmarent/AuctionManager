/*
 * Test the Field_Value class.
 *
 * $Id: mnslp_fields.cpp 2014-11-28 10:16:00 amarentes $
 * $HeadURL: https://./test/mnslp_fields_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "FieldValue.h"


using namespace auction;

class FieldValue_Test;

/*
 * We use a subclass for testing and make the test case a friend. This
 * way the test cases have access to protected methods and they don't have
 * to be public in mnslp_ipfix_fields.
 */
class fieldvalue_test : public FieldValue {
  public:
	fieldvalue_test()
		: FieldValue( ) { }

	friend class FieldValue_Test;
};


class FieldValue_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( FieldValue_Test );

	CPPUNIT_TEST( testGetters );
    CPPUNIT_TEST( testFieldValues );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testGetters();
	void testFieldValues();

  private:
    
    FieldValue *ptrField1;
    FieldValue *ptrField2;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( FieldValue_Test );




void FieldValue_Test::setUp() 
{
	ptrField1 = new FieldValue();
	ptrField2 = new FieldValue("UInt8","4");
}

void FieldValue_Test::tearDown() 
{
	delete(ptrField1);
	delete(ptrField2);
}

void FieldValue_Test::testGetters() 
{
	int lenght;
	
	lenght = ptrField1->getLen();
	CPPUNIT_ASSERT( lenght == 0 );
	
	ptrField2 = new FieldValue("UInt8","4");
	
	*ptrField1 = *ptrField2;
	CPPUNIT_ASSERT( (unsigned char) 4 == 
			(unsigned char) ParserFcts::parseULong(ptrField1->getValue()) );
	
	FieldValue * ptrField3 = new FieldValue(*ptrField2);
	CPPUNIT_ASSERT( (unsigned char) 4 == 
			(unsigned char) ParserFcts::parseULong(ptrField3->getValue()) );
		
	string type = ptrField3->getType();
	
	CPPUNIT_ASSERT(type.compare("UInt8")  == 0 );

	delete(ptrField3);

}


void FieldValue_Test::testFieldValues()
{

	delete(ptrField2);

	// UInt8
	ptrField2 = new FieldValue("UInt8","4");
	CPPUNIT_ASSERT( (unsigned char) 4 == 
			(unsigned char) ParserFcts::parseULong(ptrField2->getValue()) );
	delete(ptrField2);

	// SInt8
	ptrField2 = new FieldValue("SInt8","6");
	CPPUNIT_ASSERT( (char) 6 == 
			(char) ParserFcts::parseULong(ptrField2->getValue()));
	delete(ptrField2);


	// UInt16
	ptrField2 = new FieldValue("UInt16","16");
	CPPUNIT_ASSERT( (unsigned short) 16 == 
			(unsigned short) ParserFcts::parseULong(ptrField2->getValue()));
	delete(ptrField2);
	
	// SInt16
	ptrField2 = new FieldValue("SInt16","-18");
	CPPUNIT_ASSERT( (short) -18 == 
			(short) ParserFcts::parseLong(ptrField2->getValue()));
	delete(ptrField2);

	// UInt32
	ptrField2 = new FieldValue("UInt32","18");
	CPPUNIT_ASSERT( (short) 18 == 
			(short) ParserFcts::parseLong(ptrField2->getValue()));
	delete(ptrField2);

	// SInt32
	ptrField2 = new FieldValue("SInt32","-18");
	CPPUNIT_ASSERT( (long) -18 == 
			(long) ParserFcts::parseULong(ptrField2->getValue()));
	delete(ptrField2);

	// Binary
	ptrField2 = new FieldValue("Binary","yes");
	CPPUNIT_ASSERT( (int) 1 == 
			(int) ParserFcts::parseBool(ptrField2->getValue()));

}


