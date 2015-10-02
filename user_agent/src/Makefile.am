AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = netagent

if ENABLE_DEBUG
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-I$(top_srcdir)/foundation/include \
				-g -fno-inline -D_GLIBCXX_DEBUG -DDEBUG -DPROFILING
else
  AM_CXXFLAGS = -O2  
				-I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-I$(top_srcdir)/foundation/include
endif

netagent_SOURCES =  @top_srcdir@/foundation/src/Constants.cpp \
					@top_srcdir@/foundation/src/ConstantsAum.cpp \
					@top_srcdir@/foundation/src/ParserFcts.cpp \
					@top_srcdir@/foundation/src/XMLParser.cpp \
					@top_srcdir@/foundation/src/Timeval.cpp \
					@top_srcdir@/foundation/src/AuctionTimer.cpp \
					@top_srcdir@/foundation/src/Field.cpp \
					@top_srcdir@/foundation/src/FieldValue.cpp \
					@top_srcdir@/foundation/src/FieldDefParser.cpp \
					@top_srcdir@/foundation/src/FieldValParser.cpp \
					@top_srcdir@/foundation/src/MAPIIpApMessageParser.cpp \
					@top_srcdir@/foundation/src/BidIdSource.cpp \
					@top_srcdir@/foundation/src/AuctionIdSource.cpp \
					@top_srcdir@/foundation/src/Auction.cpp \
					@top_srcdir@/foundation/src/AuctionFileParser.cpp \
					@top_srcdir@/foundation/src/MAPIAuctionParser.cpp \
					@top_srcdir@/foundation/src/AuctionManagerInfo.cpp \
					@top_srcdir@/foundation/src/AuctionManagerComponent.cpp \
					@top_srcdir@/foundation/src/Event.cpp \
					@top_srcdir@/foundation/src/EventScheduler.cpp \
					@top_srcdir@/foundation/src/Bid.cpp \
					@top_srcdir@/foundation/src/BidFileParser.cpp \
					@top_srcdir@/foundation/src/MAPIBidParser.cpp \
					@top_srcdir@/foundation/src/BidManager.cpp \
					@top_srcdir@/foundation/src/CommandLineArgs.cpp \
					@top_srcdir@/foundation/src/ConfigManager.cpp \
					@top_srcdir@/foundation/src/ConfigParser.cpp \
					@top_srcdir@/foundation/src/Allocation.cpp \
					@top_srcdir@/foundation/src/AuctionManager.cpp \
					@top_srcdir@/foundation/src/AllocationIdSource.cpp \
					@top_srcdir@/foundation/src/MAPIAllocationParser.cpp \
					@top_srcdir@/foundation/src/AllocationManager.cpp \
					@top_srcdir@/foundation/src/AnslpClient.cpp \
					ResourceRequest.cpp \
					ResourceRequestIdSource.cpp \
					ResourceRequestFileParser.cpp \
					MAPIResourceRequestParser.cpp \
					ResourceRequestManager.cpp \
					EventSchedulerAgent.cpp \
					PageRepository.cpp \
					CtrlComm.cpp \
					AgentManagerInfo.cpp \
					AgentProcessor.cpp \
					ConstantsAgent.cpp \
					Agent.cpp \
					main.cpp

netagent_CPPFLAGS = -I$(top_srcdir)/user_agent/include \
					-I$(top_srcdir)/lib/getopt_long \
					-I$(top_srcdir)/lib/httpd \
					$(LIBGIST_CFLAGS) $(LIBPROT_CFLAGS) $(LIBFASTQUEUE_CFLAGS) \
					$(LIBIPAP_CFLAGS) $(LIBANSLP_MSG_CFLAGS) $(LIBANSLP_CFLAGS) 

netagent_LDADD = @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ $(LIBIPAP_LIBS) \
				 $(LIBPROT_LIBS) $(LIBFASTQUEUE_LIBS) $(LIBGIST_LIBS) \
				 $(LIBANSLP_MSG_LIBS) $(LIBANSLP_LIBS) \
				 $(top_builddir)/lib/getopt_long/libgetopt_long.a \
				 $(top_builddir)/lib/httpd/libhttpd.a

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -g -I@top_srcdir@/foundation/include 

AM_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lcppunit \
			 @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ $(LIBIPAP_LIBS) \
			 $(LIBPROT_LIBS) $(LIBFASTQUEUE_LIBS) $(LIBGIST_LIBS) \
			 $(LIBANSLP_MSG_LIBS) $(LIBANSLP_LIBS) \
			 $(top_builddir)/lib/getopt_long/libgetopt_long.a \
			 $(top_builddir)/lib/httpd/libhttpd.a
			 
LDADD = $(LIBIPAP_LIBS)
