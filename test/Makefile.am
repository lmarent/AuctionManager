AUTOMAKE_OPTIONS = foreign

# Rules for the test code (use `make check` to execute)
TESTS = test_runner
check_PROGRAMS = $(TESTS)

test_runner_SOURCES = 	@top_srcdir@/src/Error.cpp \
						@top_srcdir@/src/ParserFcts.cpp \
						@top_srcdir@/src/FieldValue.cpp \
						@top_srcdir@/src/XMLParser.cpp \
						@top_srcdir@/src/Constants.cpp \
						@top_srcdir@/src/ConstantsAum.cpp \
						@top_srcdir@/src/Logger.cpp \
						@top_srcdir@/src/FieldDefParser.cpp \
						@top_srcdir@/src/FieldValParser.cpp \
						@top_srcdir@/src/Bid.cpp \
						@top_srcdir@/src/BidIdSource.cpp \
						@top_srcdir@/src/AuctionIdSource.cpp \
						@top_srcdir@/src/BidFileParser.cpp \
						@top_srcdir@/src/MAPIBidParser.cpp \
						@top_srcdir@/src/MAPIAuctionParser.cpp \
						@top_srcdir@/src/ConfigParser.cpp \
						@top_srcdir@/src/Event.cpp \
						@top_srcdir@/src/Timeval.cpp \
						@top_srcdir@/src/Auction.cpp \
						@top_srcdir@/src/AuctionFileParser.cpp \
						@top_srcdir@/src/AuctionManagerInfo.cpp \
						@top_srcdir@/src/AuctionManagerComponent.cpp \
						@top_srcdir@/src/EventScheduler.cpp \
						@top_srcdir@/src/BidManager.cpp \
						@top_srcdir@/src/CommandLineArgs.cpp \
						@top_srcdir@/src/PageRepository.cpp \
						@top_srcdir@/src/CtrlComm.cpp \
						@top_srcdir@/src/ProcModule.cpp \
						@top_srcdir@/src/Module.cpp \
						@top_srcdir@/src/ModuleLoader.cpp \
						@top_srcdir@/src/AuctionTimer.cpp \
						@top_srcdir@/src/AUMProcessor.cpp \
						@top_srcdir@/src/ConfigManager.cpp \
						@top_srcdir@/src/AuctionManager.cpp \
						@top_srcdir@/src/Auctioner.cpp \
						@top_srcdir@/test/FieldValue_test.cpp \
						@top_srcdir@/test/FieldDefParser_test.cpp \
						@top_srcdir@/test/FieldValParser_test.cpp \
						@top_srcdir@/test/AuctionIdSource_test.cpp \
						@top_srcdir@/test/BidFileParser_test.cpp \
						@top_srcdir@/test/AuctionFileParser_test.cpp \
						@top_srcdir@/test/Bid_test.cpp \
						@top_srcdir@/test/AuctionTimer_test.cpp \
						@top_srcdir@/test/Bid_Manager_test.cpp \
						@top_srcdir@/test/AuctionManager_test.cpp \
						@top_srcdir@/test/test_runner.cpp

if ENABLE_DEBUG
  AM_CXXFLAGS = -g -I@top_srcdir@/include $(CPPUNIT_CFLAGS) \
			    -I$(top_srcdir)/lib/getopt_long -I$(top_srcdir)/lib/httpd \
			    -fno-inline -D_GLIBCXX_DEBUG -DDEBUG -DPROFILING -DINTEL
else
  AM_CXXFLAGS = -O2 -I@top_srcdir@/include $(CPPUNIT_CFLAGS) \
			    -I$(top_srcdir)/lib/getopt_long -I$(top_srcdir)/lib/httpd 
endif
			     
AM_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lcppunit $(top_builddir)/lib/getopt_long/libgetopt_long.a \
			 $(top_builddir)/lib/httpd/libhttpd.a \
			 @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@
LDADD = 
