/*
 * Test the MAPIAllocationParser class.
 *
 * $Id: MAPIAllocationParser_test.cpp 2015-10-01 10:09:00 amarentes $
 * $HeadURL: https://./test/MAPIAllocationParser_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "MAPIAllocationParser.h"


using namespace auction;

class MAPIAllocationParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( MAPIAllocationParser_Test );

    CPPUNIT_TEST( testMapiAllocationParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testMapiAllocationParser();
	void createAllocations(fieldDefList_t *fieldDefs);
	void loadFieldDefs(fieldDefList_t *fieldList);
	void loadFieldVals(fieldValList_t *fieldValList);
	
  private:
    
    Allocation *ptrAllocation1;
    Allocation *ptrAllocation2;
    Allocation *ptrAllocation3;
    Allocation *ptrAllocation4;
    Allocation *ptrAllocation5;

    FieldDefParser *ptrFieldParsers;
    FieldValParser *ptrFieldValParser;    
    
    MAPIAllocationParser *ptrMapiAllocationParser;
    
    
   //! filter definitions
    fieldDefList_t fieldDefs;

    //! filter values
    fieldValList_t fieldVals;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( MAPIAllocationParser_Test );

void MAPIAllocationParser_Test::createAllocations(fieldDefList_t *fieldDefs)
{

	try
	{

		// Build two allocations.

		string allSet1 = "general";
		string allName1 = "1";

		string allSet2 = "general";
		string allName2 = "2";
				
		string aSet = "general";
		string aName = "1";

		string bSet1 = "Agent1";
		string bName1 = "1";

		string bSet2 = "Agent2";
		string bName2 = "1";
		
		auction::fieldList_t fields;		
		field_t field1;
		
		auction::fieldDefListIter_t iter; 
		iter = fieldDefs->find("quantity");
		field1.name = iter->second.name;
		field1.len = iter->second.len;
		field1.type = iter->second.type;
		string fvalue = "2";
		field1.parseFieldValue(fvalue);
				
		field_t field2;
		iter = fieldDefs->find("unitprice");
		field2.name = iter->second.name;
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

		ptrAllocation1 = new Allocation(aSet, aName, bSet1, bName1, allSet1, allName1, fields, intervals);		
		
		
		ptrAllocation2 = new Allocation(aSet, aName, bSet2, bName2, allSet2, allName2,fields, intervals);


	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}

}

void MAPIAllocationParser_Test::setUp() 
{
		
	try
	{
		
		// load the filter def list
		loadFieldDefs(&fieldDefs);
	
		// load the filter val list
		loadFieldVals(&fieldVals);
		
		ptrMapiAllocationParser = new MAPIAllocationParser();
		
		
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void MAPIAllocationParser_Test::tearDown() 
{
	saveDelete(ptrFieldParsers);
    saveDelete(ptrFieldValParser);
	saveDelete(ptrMapiAllocationParser);
}


void MAPIAllocationParser_Test::testMapiAllocationParser() 
{
	try
	{

		AllocationIdSource *idSource = new AllocationIdSource(1);

		ipap_message *messageOut = new ipap_message();

		createAllocations(&fieldDefs);
		allocationDB_t *allocations = new allocationDB_t();
		allocations->push_back(ptrAllocation1);
		allocations->push_back(ptrAllocation2);
				
		// Add both allocations in the manager

		vector<ipap_message *> vct_message = 
				ptrMapiAllocationParser->get_ipap_messages(&fieldDefs, allocations);
		
		CPPUNIT_ASSERT( vct_message.size() == 2 );
		
		allocationDB_t *allocations1 = new allocationDB_t();
		ptrMapiAllocationParser->parse(&fieldDefs, &fieldVals,
									   vct_message[0],allocations1, idSource, messageOut );
											   
		CPPUNIT_ASSERT( allocations1->size() == 1 );
		ptrAllocation3 = new Allocation(*((*allocations1)[0]));
		saveDelete(allocations1);
															
		allocationDB_t *allocations2 = new allocationDB_t();
		ptrMapiAllocationParser->parse(&fieldDefs, &fieldVals,
									   vct_message[1],allocations2, idSource, messageOut );
											   
		CPPUNIT_ASSERT( allocations2->size() == 1 );
		ptrAllocation4 = new Allocation(*((*allocations2)[0]));
		saveDelete(allocations2);
					
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}

	
}


void MAPIAllocationParser_Test::loadFieldDefs(fieldDefList_t *fieldList)
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

void MAPIAllocationParser_Test::loadFieldVals(fieldValList_t *fieldValList)
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



