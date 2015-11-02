AUTOMAKE_OPTIONS = foreign

# Rules for the test code (use `make check` to execute)
TESTS = test_runner
check_PROGRAMS = $(TESTS)

test_runner_SOURCES = 	@top_srcdir@/foundation/src/Constants.cpp \
						@top_srcdir@/foundation/src/ConstantsAum.cpp \
						@top_srcdir@/foundation/src/ParserFcts.cpp \
						@top_srcdir@/foundation/src/XMLParser.cpp \
						@top_srcdir@/foundation/src/Timeval.cpp \
						@top_srcdir@/foundation/src/AuctionTimer.cpp \
						@top_srcdir@/foundation/src/Field.cpp \
						@top_srcdir@/foundation/src/FieldValue.cpp \
						@top_srcdir@/foundation/src/FieldDefParser.cpp \
						@top_srcdir@/foundation/src/FieldValParser.cpp \
						@top_srcdir@/foundation/src/FieldDefManager.cpp \
						@top_srcdir@/foundation/src/IpApMessageParser.cpp \
						@top_srcdir@/foundation/src/TemplateIdSource.cpp \
						@top_srcdir@/foundation/src/IdSource.cpp \
						@top_srcdir@/foundation/src/MessageIdSource.cpp \
						@top_srcdir@/foundation/src/AuctioningObject.cpp \
						@top_srcdir@/foundation/src/Auction.cpp \
						@top_srcdir@/foundation/src/AuctionFileParser.cpp \
						@top_srcdir@/foundation/src/MAPIAuctionParser.cpp \
						@top_srcdir@/foundation/src/AuctionManager.cpp \
						@top_srcdir@/foundation/src/AuctionManagerInfo.cpp \
						@top_srcdir@/foundation/src/AuctionManagerComponent.cpp \
						@top_srcdir@/foundation/src/Event.cpp \
						@top_srcdir@/foundation/src/EventScheduler.cpp \
						@top_srcdir@/foundation/src/BiddingObject.cpp \
						@top_srcdir@/foundation/src/BiddingObjectFileParser.cpp \
						@top_srcdir@/foundation/src/MAPIBiddingObjectParser.cpp \
						@top_srcdir@/foundation/src/BiddingObjectManager.cpp \
						@top_srcdir@/foundation/src/CommandLineArgs.cpp \
						@top_srcdir@/foundation/src/ConfigManager.cpp \
						@top_srcdir@/foundation/src/ConfigParser.cpp \
						@top_srcdir@/foundation/test/Timeval_test.cpp \
						@top_srcdir@/foundation/test/FieldValue_test.cpp \
						@top_srcdir@/foundation/test/FieldDefParser_test.cpp \
						@top_srcdir@/foundation/test/AuctionTimer_test.cpp \
						@top_srcdir@/foundation/test/FieldValParser_test.cpp \
						@top_srcdir@/foundation/test/MessageIdSource_test.cpp \
						@top_srcdir@/foundation/test/BiddingObjectFileParser_test.cpp \
						@top_srcdir@/foundation/test/BiddingObject_test.cpp \
						@top_srcdir@/foundation/test/AuctionFileParser_test.cpp \
						@top_srcdir@/foundation/test/Auction_test.cpp \
						@top_srcdir@/foundation/test/MAPIAuctionParser_test.cpp \
						@top_srcdir@/foundation/test/MAPIBiddingObjectParser_test.cpp \
						@top_srcdir@/foundation/test/BiddingObjectManager_test.cpp \
						@top_srcdir@/foundation/test/AuctionManager_test.cpp \
						@top_srcdir@/foundation/test/test_runner.cpp




if ENABLE_DEBUG
  AM_CXXFLAGS = -g -I@top_srcdir@/foundation/include $(CPPUNIT_CFLAGS) \
			    -I$(top_srcdir)/lib/getopt_long -I$(top_srcdir)/lib/httpd \
			    -fno-inline -D_GLIBCXX_DEBUG -DDEBUG -DPROFILING -DINTEL \
			     $(LIBGIST_CFLAGS) $(LIBPROT_CFLAGS) $(LIBFASTQUEUE_CFLAGS) \
				 $(LIBIPAP_CFLAGS) $(LIBANSLP_MSG_CFLAGS) $(LIBANSLP_CFLAGS) 
else
  AM_CXXFLAGS = -O2 -I@top_srcdir@/foundation/include $(CPPUNIT_CFLAGS) \
			    -I$(top_srcdir)/lib/getopt_long -I$(top_srcdir)/lib/httpd \
			     $(LIBGIST_CFLAGS) $(LIBPROT_CFLAGS) $(LIBFASTQUEUE_CFLAGS) \
				 $(LIBIPAP_CFLAGS) $(LIBANSLP_MSG_CFLAGS) $(LIBANSLP_CFLAGS) 
endif
			     
AM_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lcppunit $(top_builddir)/lib/getopt_long/libgetopt_long.a \
			 $(top_builddir)/lib/httpd/libhttpd.a \
			 @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ $(LIBIPAP_LIBS) \
			 $(LIBPROT_LIBS) $(LIBFASTQUEUE_LIBS) $(LIBGIST_LIBS) \
			 $(LIBANSLP_MSG_LIBS) $(LIBANSLP_LIBS)
			 
			 	 
LDADD = $(LIBIPAP_LIBS)
