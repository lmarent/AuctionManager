/*
 * test_runner.cpp - Run the test suite.
 *
 * $Id: test_runner.cpp 2915 2008-02-26 12:29:28Z bless $
 * $HeadURL: https://svn.ipv6.tm.uka.de/nsis/ntlp/trunk/test/test_runner.cpp $
 */
#include <cstdlib> // for getenv()
#include <string>

#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include "logfile.h"
#include "AnslpClient.h"


using namespace CppUnit;

// global timeout flag
int g_timeout = 0;

protlib::log::logfile commonlog("Auctioneer.log", anslp::anslp_config::USE_COLOURS);
protlib::log::logfile &protlib::log::DefaultLog(commonlog);

/*
 * Run the test suite, return 0 on success, 1 on error.
 */
int main(void) {


	TestResult controller;

	TestResultCollector result;
	controller.addListener(&result);

	BriefTestProgressListener progress;
	controller.addListener(&progress);

	TestRunner runner;
	runner.addTest( TestFactoryRegistry::getRegistry().makeTest() );
	runner.run(controller);

	CompilerOutputter outputter(&result, std::cerr);
	outputter.write();

	return result.wasSuccessful() ? 0 : 1;
}
