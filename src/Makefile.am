AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = auctionManager

if ENABLE_DEBUG
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long -I$(top_srcdir)/lib/httpd \
				-g -fno-inline -D_GLIBCXX_DEBUG -DDEBUG -DPROFILING
else
  AM_CXXFLAGS = -O2  
				-I$(top_srcdir)/lib/getopt_long -I$(top_srcdir)/lib/httpd 
endif

auctionManager_SOURCES =  Error.cpp \
						  Event.cpp \
						  EventScheduler.cpp \
						  Logger.cpp \
						  Timeval.cpp \
						  FieldDefParser.cpp \
						  FieldValParser.cpp \
						  FieldValue.cpp \
						  MAPIBidParser.cpp \
						  Module.cpp \
						  ModuleLoader.cpp \
						  PageRepository.cpp \
						  ParserFcts.cpp \
						  ProcModule.cpp \
						  Constants.cpp \
						  ConstantsAum.cpp \
						  ConfigParser.cpp \
						  ConfigManager.cpp \
						  CommandLineArgs.cpp \
						  CtrlComm.cpp \
						  XMLParser.cpp \
						  Bid.cpp \
						  AuctionIdSource.cpp \
						  Auction.cpp \
						  AuctionManager.cpp \
						  AuctionFileParser.cpp \
						  MAPIAuctionParser.cpp \
						  BidIdSource.cpp \
						  BidManager.cpp \
						  BidFileParser.cpp \
						  AUMProcessor.cpp \
						  AuctionManagerInfo.cpp \
						  AuctionTimer.cpp \
						  AuctionManagerComponent.cpp \
						  Auctioner.cpp \
						  main.cpp

auctionManager_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/lib/getopt_long \
						  -I$(top_srcdir)/lib/httpd

auctionManager_LDADD = $(top_builddir)/lib/getopt_long/libgetopt_long.a \
					   $(top_builddir)/lib/httpd/libhttpd.a \
					   @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -g -I@top_srcdir@/include @poco_CFLAGS@ @NL3_CFLAGS@ @NL_ROUTE_3_CFLAGS@
AM_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lcppunit $(top_builddir)/lib/getopt_long/libgetopt_long.a \
			 $(top_builddir)/lib/httpd/libhttpd.a \
			 @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@
			 
LDADD = @poco_LIBS@ 
