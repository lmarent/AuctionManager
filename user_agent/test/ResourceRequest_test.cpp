/*
 * Test the ResourceRequest class.
 *
 * $Id: ResourceRequest_test.cpp 2015-08-26 09:38:00 amarentes $
 * $HeadURL: https://./test/ResourceRequest_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "ResourceRequest.h"
#include "FieldValue.h"
#include "FieldDefParser.h"
#include "ResourceRequestIdSource.h"

using namespace auction;

class ResourceRequest_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( ResourceRequest_Test );

    CPPUNIT_TEST( testResourceRequets );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();

	void testResourceRequets();
	void loadFieldDefs(fieldDefList_t *fieldList);
	

  private:
    
    ResourceRequest *ptrresourceRequest1;
    ResourceRequest *ptrresourceRequest2;
    FieldDefParser *ptrFieldParsers;
        
    //! filter definitions
    auction::fieldDefList_t fieldDefs;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( ResourceRequest_Test );


void ResourceRequest_Test::setUp() 
{
		
	try
	{
		// load the filter def list
		loadFieldDefs(&fieldDefs);
						
	}
	catch(Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void ResourceRequest_Test::tearDown() 
{
	
	delete(ptrFieldParsers);
	delete(ptrresourceRequest1);
	delete(ptrresourceRequest2);	
	
}

void ResourceRequest_Test::testResourceRequets() 
{

	time_t              now = time(NULL);
	
	auction::fieldList_t fields;
	resourceReqIntervalList_t intervals;
	
	field_t field1;
	field_t field2;
	field_t field3;
	
	auction::fieldDefListIter_t iter; 
	iter = fieldDefs.find("quantity");
	if (iter != fieldDefs.end()){
		field1.name = iter->second.name;
		field1.len = iter->second.len;
		field1.type = iter->second.type;
		string fvalue1 = "2";
		field1.parseFieldValue(fvalue1);
	} else {
		throw Error("field quantity sot found");
	}

	iter = fieldDefs.find("unitprice");
	if (iter != fieldDefs.end()){
		field2.name = iter->second.name;
		field2.len = iter->second.len;
		field2.type = iter->second.type;
		string fvalue2 = "0.012";
		field2.parseFieldValue(fvalue2);
	} else {
		throw Error("field unitprice sot found");
	}
	
	iter = fieldDefs.find("unitbudget");
	if (iter != fieldDefs.end()){
		field3.name = iter->second.name;
		field3.len = iter->second.len;
		field3.type = iter->second.type;
		string fvalue3 = "0.04";
		field3.parseFieldValue(fvalue3);
	} else {
		throw Error("field unitbudget sot found");
	}

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
	
	string set = "Group1";
	string name = "1";
	
	ptrresourceRequest1 = new ResourceRequest(set, name, fields, intervals);

	auction::fieldList_t fields1;
	resourceReqIntervalList_t intervals1;

	field_t field11;
	field_t field21;
	field_t field31;
	
	iter = fieldDefs.find("quantity");
	if (iter != fieldDefs.end()){
		field11.name = iter->second.name;
		field11.len = iter->second.len;
		field11.type = iter->second.type;
		string fvalue4 = "3";
		field11.parseFieldValue(fvalue4);
	} else {
		throw Error("field quantity sot found");
	}
	
	iter = fieldDefs.find("unitprice");
	if (iter != fieldDefs.end()){
		field21.name = iter->second.name;
		field21.len = iter->second.len;
		field21.type = iter->second.type;
		string fvalue5 = "0.013";
		field21.parseFieldValue(fvalue5);
	} else {
		throw Error("field unitprice sot found");
	}
	
	iter = fieldDefs.find("unitbudget");
	if (iter != fieldDefs.end()){
		field31.name = iter->second.name;
		field31.len = iter->second.len;
		field31.type = iter->second.type;
		string fvalue6 = "0.05";
		field31.parseFieldValue(fvalue6);
	} else {
		throw Error("field unitbudget sot found");
	}

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
	
	ptrresourceRequest2 = new ResourceRequest(set, name, fields1, intervals1);
	
	CPPUNIT_ASSERT( ptrresourceRequest1->getResourceRequestSet().compare(ptrresourceRequest2->getResourceRequestSet())  == 0  ); 
	CPPUNIT_ASSERT( ptrresourceRequest1->getResourceRequestName().compare(ptrresourceRequest2->getResourceRequestName())  == 0  ); 
	
	CPPUNIT_ASSERT( ptrresourceRequest1->getState() == ptrresourceRequest2->getState()  ); 
	
	ptrresourceRequest1->setState(AO_VALID);
	
	CPPUNIT_ASSERT( ptrresourceRequest1->getState() == AO_VALID  );
	
	//cout << "Info:" << ptrresourceRequest1->getInfo() << endl;
	
	CPPUNIT_ASSERT( ptrresourceRequest1->getFields()->size() == 3  );
	CPPUNIT_ASSERT( ptrresourceRequest1->getIntervals()->size() == 3  );
	
}

void ResourceRequest_Test::loadFieldDefs(fieldDefList_t *fieldList)
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





