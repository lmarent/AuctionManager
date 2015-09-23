/*
 * Test the ResourceRequest Manager class.
 *
 * $Id: ResourceRequestManager_test.cpp 2015-08-26 11:30:00 amarentes $
 * $HeadURL: https://./test/ResourceRequestManager_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ResourceRequest.h"
#include "FieldValue.h"
#include "FieldDefParser.h"
#include "ResourceRequestIdSource.h"
#include "ResourceRequestFileParser.h"
#include "ResourceRequestManager.h"

using namespace auction;

class ResourceRequestManager_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( ResourceRequestManager_Test );

    CPPUNIT_TEST( testResourceManagerManager );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void createResourceRequest();
	void testResourceManagerManager();
	void loadFieldDefs(fieldDefList_t *fieldList);
	
  private:
    
    ResourceRequest *ptrResourceRequest1;
    ResourceRequest *ptrResourceRequest2;
    
    auto_ptr<EventSchedulerAgent>  evnt;
    ResourceRequestManager *manager;
    
    //! filter definitions
    fieldDefList_t fieldDefs;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( ResourceRequestManager_Test );

void ResourceRequestManager_Test::createResourceRequest()
{
	try
	{

		time_t              now = time(NULL);
		
		fieldList_t fields;
		resourceReqIntervalList_t intervals;
		
		field_t field1;
		field_t field2;
		field_t field3;
		
		fieldDefListIter_t iter; 
		iter = manager->getFieldDef()->find("quantity");
		field1.name = iter->second.name;
		field1.len = iter->second.len;
		field1.type = iter->second.type;
		string fvalue1 = "2";
		field1.parseFieldValue(fvalue1);

		iter = manager->getFieldDef()->find("maxvalue");
		field2.name = iter->second.name;
		field2.len = iter->second.len;
		field2.type = iter->second.type;
		string fvalue2 = "0.35";
		field2.parseFieldValue(fvalue2);
		
		iter = manager->getFieldDef()->find("budget");
		field3.name = iter->second.name;
		field3.len = iter->second.len;
		field3.type = iter->second.type;
		string fvalue3 = "0.04";
		field3.parseFieldValue(fvalue3);

		fields.push_back(field1);
		fields.push_back(field2);
		fields.push_back(field3);
		
		resourceReq_interval_t interval1;
		resourceReq_interval_t interval2;
		resourceReq_interval_t interval3;

		interval1.start = now;
		interval1.stop = interval1.start + 100;
		interval1.interval = 100;
		interval1.align = 0;
		
		interval2.start = now;
		interval2.stop = interval1.start + 300;
		interval2.interval = 200;
		interval2.align = 0;

		interval3.start = now;
		interval3.stop = interval1.start + 250;
		interval3.interval = 150;
		interval3.align = 0;
		
		intervals.push_back(interval1);
		intervals.push_back(interval2);
		intervals.push_back(interval3);
		
		string set1 = "set1";
		string name1 = "1";
		
		ptrResourceRequest1 = new ResourceRequest(set1, name1, fields, intervals);
		ptrResourceRequest1->setState(RR_VALID);
		

		fieldList_t fields1;
		resourceReqIntervalList_t intervals1;

		field_t field11;
		field_t field21;
		field_t field31;
		
		iter = manager->getFieldDef()->find("quantity");
		field11.name = iter->second.name;
		field11.len = iter->second.len;
		field11.type = iter->second.type;
		string fvalue4 = "3";
		field11.parseFieldValue(fvalue4);

		iter = manager->getFieldDef()->find("unitprice");
		field21.name = iter->second.name;
		field21.len = iter->second.len;
		field21.type = iter->second.type;
		string fvalue5 = "0.013";
		field21.parseFieldValue(fvalue5);
		
		iter = manager->getFieldDef()->find("budget");
		field31.name = iter->second.name;
		field31.len = iter->second.len;
		field31.type = iter->second.type;
		string fvalue6 = "0.05";
		field31.parseFieldValue(fvalue6);

		fields1.push_back(field11);
		fields1.push_back(field21);
		fields1.push_back(field31);
		
		resourceReq_interval_t interval11;
		resourceReq_interval_t interval21;
		resourceReq_interval_t interval31;

		interval11.start = now;
		interval11.stop = interval1.start + 300;
		interval11.interval = 50;
		interval11.align = 0;
		
		interval21.start = now;
		interval21.stop = interval1.start + 400;
		interval21.interval = 150;
		interval21.align = 0;

		interval31.start = now;
		interval31.stop = interval1.start + 250;
		interval31.interval = 150;
		interval31.align = 0;
		
		intervals1.push_back(interval11);
		intervals1.push_back(interval21);
		intervals1.push_back(interval31);

		string set2 = "set1";
		string name2 = "2";		
		ptrResourceRequest2 = new ResourceRequest(set2, name2, fields1, intervals1);
						
			
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}


void ResourceRequestManager_Test::setUp() 
{
		
	try
	{

		string fieldname = DEF_SYSCONFDIR "/fielddef.xml";
		manager = new ResourceRequestManager(fieldname);
		
		auto_ptr<EventSchedulerAgent> _evnt(new EventSchedulerAgent());
        evnt = _evnt;
		
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void ResourceRequestManager_Test::tearDown() 
{
	saveDelete(manager);
	evnt.release();
	
}

void ResourceRequestManager_Test::testResourceManagerManager() 
{

	try
	{
		
		string filename = DEF_SYSCONFDIR "/example_resource_request1.xml";
		resourceRequestDB_t *new_requests = manager->parseResourceRequests(filename);
		manager->addResourceRequests(new_requests, evnt.get()); 
		CPPUNIT_ASSERT( manager->getNumResourceRequests() == 1 );

		// Release all resource request.
		saveDelete(manager);

		string fieldname = DEF_SYSCONFDIR "/fielddef.xml";
		manager = new ResourceRequestManager(fieldname);

		createResourceRequest();
		
		// Add both Resource Allocations in the manager
		manager->addResourceRequest(ptrResourceRequest1);
		manager->addResourceRequest(ptrResourceRequest2);
		
		CPPUNIT_ASSERT( manager->getNumResourceRequests() == 2 );
				
		ResourceRequest *tmpResourceRequest = manager->getResourceRequest(0);
				
		CPPUNIT_ASSERT( ptrResourceRequest1->getInfo() == manager->getInfo("set1", "1") );
		
		CPPUNIT_ASSERT( manager->getResourceRequest(1) != NULL );
		
		manager->delResourceRequest(1, evnt.get() );

		CPPUNIT_ASSERT( manager->getNumResourceRequests() == 1 );
		
		manager->delResourceRequest("set1", "1", evnt.get() );

		CPPUNIT_ASSERT( manager->getNumResourceRequests() == 0 );
				
		// Release all resource request.
		saveDelete(manager);
								
		manager = new ResourceRequestManager(fieldname);
		
		createResourceRequest();

		manager->addResourceRequest(ptrResourceRequest1);
		manager->addResourceRequest(ptrResourceRequest2);
				
		CPPUNIT_ASSERT( manager->getNumResourceRequests() == 2 );
				
		// Release all allocations.
		saveDelete(manager);
				
		manager = new ResourceRequestManager(fieldname);
		
		createResourceRequest();

		manager->addResourceRequest(ptrResourceRequest1);
		manager->addResourceRequest(ptrResourceRequest2);
		
		ResourceRequest *tmpResourceRequest2 = manager->getResourceRequest("set1", "1");
				
		CPPUNIT_ASSERT( tmpResourceRequest2->getFields()->size() == 3 );

		// Release all ResourceRequest.
		saveDelete(manager);
				
		manager = new ResourceRequestManager(fieldname);
		
		createResourceRequest();
		
		resourceRequestDB_t requests;
		
		requests.push_back(ptrResourceRequest1);
		requests.push_back(ptrResourceRequest2);
		manager->addResourceRequests(&requests, evnt.get());
		
		CPPUNIT_ASSERT( manager->getNumResourceRequests() == 2 );
		
		manager->activateResourceRequests(&requests, evnt.get());

		manager->delResourceRequests(&requests, evnt.get());

		CPPUNIT_ASSERT( manager->getNumResourceRequests() == 0 );		
			
	} catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}


}





