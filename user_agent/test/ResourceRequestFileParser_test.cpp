/*
 * Test the BidFileParser class.
 *
 * $Id: BidFileParser_test.cpp 2015-07-24 15:50:00 amarentes $
 * $HeadURL: https://./test/BidFileParser_test.cpp $
 */
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ParserFcts.h"
#include "ResourceRequestFileParser.h"
#include "FieldDefParser.h"
#include "ResourceRequestIdSource.h"

using namespace auction;

class ResourceRequestFileParser_Test : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( ResourceRequestFileParser_Test );

	CPPUNIT_TEST( testParser );
	CPPUNIT_TEST_SUITE_END();

  public:
	void setUp();
	void tearDown();
	void loadFieldDefs(fieldDefList_t *fieldList);

	void testParser();

  private:
    
    ResourceRequestFileParser *ptrResourceRequestFileParser;
    FieldDefParser *ptrFieldParsers;
    ResourceRequestIdSource *idSource;

    //! filter definitions
    fieldDefList_t fieldDefs;
    
};

CPPUNIT_TEST_SUITE_REGISTRATION( ResourceRequestFileParser_Test );


void ResourceRequestFileParser_Test::setUp() 
{
	const string filename = DEF_SYSCONFDIR "/example_resource_request1.xml";

	try
	{
		ptrResourceRequestFileParser = new ResourceRequestFileParser(filename);
		idSource = new ResourceRequestIdSource(1);

		// load the filter def list
		loadFieldDefs(&fieldDefs);
	
		
	}catch (Error &e)
	{
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void ResourceRequestFileParser_Test::tearDown() 
{
	delete(ptrResourceRequestFileParser);
	delete(idSource);
    delete(ptrFieldParsers);
	
}

void ResourceRequestFileParser_Test::testParser() 
{
	resourceRequestDB_t *new_requests = new resourceRequestDB_t();
		
	try
	{

		time_t              now = time(NULL);
		
		fieldList_t fields;
		resourceReqIntervalList_t intervals;
		
		field_t field1;
		field_t field2;
		field_t field3;
		
		fieldDefListIter_t iter; 
		iter = fieldDefs.find("quantity");
		field1.name = iter->second.name;
		field1.len = iter->second.len;
		field1.type = iter->second.type;
		string fvalue1 = "2";
		field1.parseFieldValue(fvalue1);
		
		iter = fieldDefs.find("budget");
		field3.name = iter->second.name;
		field3.len = iter->second.len;
		field3.type = iter->second.type;
		string fvalue3 = "0.3";
		field3.parseFieldValue(fvalue3);

		iter = fieldDefs.find("maxvalue");
		field2.name = iter->second.name;
		field2.len = iter->second.len;
		field2.type = iter->second.type;
		string fvalue2 = "0.16";
		field2.parseFieldValue(fvalue2);

		fields.push_back(field1);
		fields.push_back(field3);
		fields.push_back(field2);
		
		resourceReq_interval_t interval1;
		resourceReq_interval_t interval2;

		interval1.start = now;
		interval1.stop = interval1.start + 100;
		interval1.interval = 0;
		interval1.align = 0;
		
		interval2.start = now;
		interval2.stop = interval1.start + 100;
		interval2.interval = 0;
		interval2.align = 0;
		
		intervals.push_back(interval1);
		intervals.push_back(interval2);
		
		string set = "set1";
		string name = "1";
		
		ResourceRequest *ptrresourceRequest1 = new ResourceRequest(set, name, fields, intervals);
		
		
		ptrResourceRequestFileParser->parse(&fieldDefs, 
											new_requests,
											idSource );
		
		CPPUNIT_ASSERT( new_requests->size() == 1 );
		
		string info1 = (*new_requests)[0]->getInfo();
		string info2 = ptrresourceRequest1->getInfo();
		
		CPPUNIT_ASSERT( info1.compare(info2) == 0);
		
		saveDelete(ptrresourceRequest1);
		
	}
	catch (Error &e){
		std::cout << "Error:" << e.getError() << std::endl << std::flush;
	}
}

void ResourceRequestFileParser_Test::loadFieldDefs(fieldDefList_t *fieldList)
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
