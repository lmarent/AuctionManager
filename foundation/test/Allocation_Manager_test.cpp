/*
 * Test the Allocation Manager class.
 *
 * $Id: Allocation_Manager_test.cpp 2015-08-21 10:50:00 amarentes $
 * $HeadURL: https://./test/Allocation_Manager_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "AllocationManager.h"

using namespace auction;

class Allocation_Manager_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( Allocation_Manager_Test );

    CPPUNIT_TEST( testAllocationManager );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testAllocationManager();
	void createAllocations();
	void loadFieldDefs(fieldDefList_t *fieldList);
	
  private:
    
    Allocation *ptrAllocation1;
    Allocation *ptrAllocation2;
    Allocation *ptrAllocation3;
    Allocation *ptrAllocation4;
    Allocation *ptrAllocation5;
    
    auto_ptr<EventScheduler>  evnt;
    AllocationManager *manager;
    
    
    //! filter definitions
    fieldDefList_t fieldDefs;
    
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( Allocation_Manager_Test );

void Allocation_Manager_Test::createAllocations()
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
		iter = manager->getFieldDef()->find("quantity");
		field1.len = iter->second.len;
		field1.type = iter->second.type;
		string fvalue = "2";
		field1.parseFieldValue(fvalue);
				
		field_t field2;
		iter = manager->getFieldDef()->find("unitprice");
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
		
		auction::fieldList_t fields2;
		allocationIntervalList_t intervals2;
		
		ptrAllocation2 = new Allocation(aSet, aName, bSet2, bName2, allSet2, allName2,fields2, intervals2);


	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}

}

void Allocation_Manager_Test::setUp() 
{
		
	try
	{

		const string fieldname = DEF_SYSCONFDIR "/fielddef.xml";
		
		manager = new AllocationManager(fieldname);
		
		auto_ptr<EventScheduler> _evnt(new EventScheduler());
        evnt = _evnt;
		
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void Allocation_Manager_Test::tearDown() 
{
	delete(manager);
	evnt.reset();
}


void Allocation_Manager_Test::testAllocationManager() 
{
	try
	{
		const string fieldname = DEF_SYSCONFDIR "/fielddef.xml";

		createAllocations();
		// Add both allocations in the manager
		
		manager->addAllocation(ptrAllocation1);
		manager->addAllocation(ptrAllocation2);
		
		CPPUNIT_ASSERT( manager->getNumAllocations() == 2 );
				
		Allocation *tmpAllocation = manager->getAllocation(0);
				
		CPPUNIT_ASSERT( ptrAllocation1->getInfo() == manager->getInfo(0) );
		
		CPPUNIT_ASSERT( manager->getAllocation(1) != NULL );
				
		manager->delAllocation(0, evnt.get() );

		CPPUNIT_ASSERT( manager->getNumAllocations() == 1 );
				
		manager->delAllocation("general", "2", evnt.get() );

		CPPUNIT_ASSERT( manager->getNumAllocations() == 0 );
				
		// Release all allocations.
		saveDelete(manager);
				
		manager = new AllocationManager(fieldname);
		
		createAllocations();

		manager->addAllocation(ptrAllocation1);
		manager->addAllocation(ptrAllocation2);
		
		CPPUNIT_ASSERT( manager->getNumAllocations() == 2 );
		
		manager->delBidAllocations("Agent1", "1", evnt.get() );
		
		CPPUNIT_ASSERT( manager->getNumAllocations() == 1 );
		
		manager->delAuctionAllocations("general", "1", evnt.get() );
		
		CPPUNIT_ASSERT( manager->getNumAllocations() == 0 );
		
		// Release all allocations.
		saveDelete(manager);
				
		manager = new AllocationManager(fieldname);
		
		createAllocations();

		manager->addAllocation(ptrAllocation1);
		manager->addAllocation(ptrAllocation2);
				
		Allocation *tmpAllocation2 = manager->getAllocation("general", "1");
		
		CPPUNIT_ASSERT( tmpAllocation2->getNumFields() == 2 );

		// Release all allocations.
		saveDelete(manager);
				
		manager = new AllocationManager(fieldname);
		
		createAllocations();
		
		allocationDB_t allocations;
		
		allocations.push_back(ptrAllocation1);
		allocations.push_back(ptrAllocation2);
		manager->addAllocations(&allocations, evnt.get());
		
		CPPUNIT_ASSERT( manager->getNumAllocations() == 2 );
		
		manager->activateAllocations(&allocations, evnt.get());

		manager->delAllocations(&allocations, evnt.get());

		CPPUNIT_ASSERT( manager->getNumAllocations() == 0 );
			
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
		throw e;
	}

	
}




