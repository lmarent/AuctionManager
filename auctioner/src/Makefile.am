AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = auctionManager

if ENABLE_DEBUG
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-I$(top_srcdir)/foundation/include \
				-g -fno-inline -D_GLIBCXX_DEBUG -DDEBUG -DPROFILING
else
  AM_CXXFLAGS = -O2 -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-I$(top_srcdir)/foundation/include
endif

auctionManager_SOURCES =  @top_srcdir@/foundation/src/Constants.cpp \
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
						  @top_srcdir@/foundation/src/TemplateIdSource.cpp \
						  @top_srcdir@/foundation/src/IpApMessageParser.cpp \
						  @top_srcdir@/foundation/src/IdSource.cpp \
						  @top_srcdir@/foundation/src/AuctioningObject.cpp \
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
						  @top_srcdir@/foundation/src/Session.cpp \
						  @top_srcdir@/foundation/src/SessionManager.cpp \
						  @top_srcdir@/foundation/src/ModuleLoader.cpp \
						  @top_srcdir@/foundation/src/Module.cpp \
						  @top_srcdir@/foundation/src/ProcModule.cpp \
						  EventSchedulerAuctioner.cpp \
						  PageRepository.cpp \
						  CtrlComm.cpp \
						  AUMProcessor.cpp \
						  Auctioner.cpp \
						  main.cpp

auctionManager_CPPFLAGS = -I$(top_srcdir)/lib/getopt_long \
						  -I$(top_srcdir)/lib/httpd \
						  -I$(top_srcdir)/auctioner/include \
						  $(LIBGIST_CFLAGS) $(LIBPROT_CFLAGS) $(LIBFASTQUEUE_CFLAGS) \
						  $(LIBIPAP_CFLAGS) $(LIBANSLP_MSG_CFLAGS) $(LIBANSLP_CFLAGS) 

auctionManager_LDADD = $(top_builddir)/lib/getopt_long/libgetopt_long.a \
					   $(top_builddir)/lib/httpd/libhttpd.a \
					   @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@

auctionManager_LDFLAGS = -export-dynamic -module \
						 $(CPPUNIT_LIBS) -ldl -lcppunit $(top_builddir)/lib/getopt_long/libgetopt_long.a \
						 $(top_builddir)/lib/httpd/libhttpd.a \
						 @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ $(LIBIPAP_LIBS) \
						 $(LIBPROT_LIBS) $(LIBFASTQUEUE_LIBS) $(LIBGIST_LIBS) \
					     $(LIBANSLP_MSG_LIBS) $(LIBANSLP_LIBS)

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -g -I@top_srcdir@/include
			 
LDADD = $(LIBIPAP_LIBS)

$(top_builddir)/lib/httpd/libhttpd.a:
	cd $(top_builddir)/lib/httpd ; $(MAKE)

$(top_builddir)/lib/getopt_long/libgetopt_long.a:
	cd $(top_builddir)/lib/getopt_long ; $(MAKE)

install-data-hook:
	$(INSTALL) -d $(DESTDIR)/$(localstatedir)/run
	$(INSTALL) -d $(DESTDIR)/$(localstatedir)/log
