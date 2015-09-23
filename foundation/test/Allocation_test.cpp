/*
 * Test the Allocation class.
 *
 * $Id: Allocation_test.cpp 2014-11-28 10:16:00 amarentes $
 * $HeadURL: https://./test/Allocation_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <config.h>
#include "Allocation.h"
#include "FieldValue.h"
#include "FieldValParser.h"
#include "FieldDefParser.h"
#include "BidIdSource.h"

using namespace auction;

class Allocation_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( Allocation_Test );

    CPPUNIT_TEST( testAllocations );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testAllocations();
	void testFieldValues();
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);
	

  private:
    
    Allocation *ptrAllocation1;
    Allocation *ptrAllocation2;
    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;    
    
    //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
        
};

CPPUNIT_TEST_SUITE_REGISTRATION( Allocation_Test );




void Allocation_Test::setUp() 
{
		
	try
	{

		// load the filter def list
		loadFieldDefs(&fieldDefs);
	
		// load the filter val list
		loadFieldVals(&fieldVals);
												
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void Allocation_Test::tearDown() 
{
	saveDelete(ptrAllocation1);
	saveDelete(ptrAllocation2);
	saveDelete(ptrFieldParsers);
	saveDelete(ptrFieldValParser);	
}

void Allocation_Test::testAllocations() 
{
	try
	{

		string aSet = "general";
		string aName = "1";

		string bSet1 = "Agent1";
		string bName1 = "1";

		string bSet2 = "Agent2";
		string bName2 = "1";
			
		fieldList_t fields;
		
		field_t field1;
		
		fieldDefListIter_t iter; 
		iter = fieldDefs.find("quantity");
		field1.len = iter->second.len;
		field1.type = iter->second.type;
		string fvalue = "2";
		field1.parseFieldValue(fvalue);
				
		field_t field2;
		iter = fieldDefs.find("unitprice");
		field2.len = iter->second.len;
		field2.type = iter->second.type;
		string fvalue2 = "0.015";
		field2.parseFieldValue(fvalue);
				
		fields.push_back(field1);
		fields.push_back(field2);
				
		allocationIntervalList_t intervals;

		alloc_interval_t interval1;
		interval1.start = time(NULL);
		interval1.stop = time(NULL);
		
		intervals.push_back(interval1);

		alloc_interval_t interval2;
		interval2.start = time(NULL);
		interval2.stop = time(NULL);
		intervals.push_back(interval2);
		
		ptrAllocation1 = new Allocation(aSet, aName, bSet1, bName1, fields, intervals);
						
		CPPUNIT_ASSERT( aSet.compare(ptrAllocation1->getAuctionSet()) == 0 );
		CPPUNIT_ASSERT( aName.compare(ptrAllocation1->getAuctionName()) == 0 );
		
		fieldList_t fields2;
		allocationIntervalList_t intervals2;
		
		ptrAllocation2 = new Allocation(aSet, aName, bSet2, bName2, fields2, intervals2);

		CPPUNIT_ASSERT( bSet2.compare(ptrAllocation2->getBidSet()) == 0 );
		CPPUNIT_ASSERT( bName2.compare(ptrAllocation2->getBidName()) == 0 );

		CPPUNIT_ASSERT( ptrAllocation1->getNumFields() == 2 );
		CPPUNIT_ASSERT( ptrAllocation1->getNumIntervals() == 2 );

	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}

}

void Allocation_Test::loadFieldDefs(fieldDefList_t *fieldList)
{
	const string filename = DEF_SYSCONFDIR "/fielddef.xml";

	try
	{
		ptrFieldParsers = new FieldDefParser(filename);
		ptrFieldParsers->parse(fieldList);
		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}

}

void Allocation_Test::loadFieldVals(fieldValList_t *fieldValList)
{
	const string filename = DEF_SYSCONFDIR "/fieldval.xml";
	try
	{
		ptrFieldValParser = new FieldValParser(filename);
		ptrFieldValParser->parse(fieldValList);
				
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}

}
