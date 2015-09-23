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

auctionManager_SOURCES =  EventSchedulerAuctioner.cpp \
						  Module.cpp \
						  ModuleLoader.cpp \
						  PageRepository.cpp \
						  ProcModule.cpp \
						  CtrlComm.cpp \
						  AUMProcessor.cpp \
						  Auctioner.cpp \
						  main.cpp

auctionManager_CPPFLAGS = -I$(top_srcdir)/lib/getopt_long \
						  -I$(top_srcdir)/lib/httpd \
						  -I$(top_srcdir)/auctioner/include $(LIBIPAP_CFLAGS)

auctionManager_LDADD = $(top_builddir)/lib/getopt_long/libgetopt_long.a \
					   $(top_builddir)/lib/httpd/libhttpd.a \
					   $(top_builddir)/foundation/src/libfoundation.la \
					   @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -g -I@top_srcdir@/include
AM_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lcppunit $(top_builddir)/lib/getopt_long/libgetopt_long.a \
			 $(top_builddir)/lib/httpd/libhttpd.a \
			 $(top_builddir)/foundation/src/libfoundation.la \
			 @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ $(LIBIPAP_LIBS)
			 
LDADD = ../foundation/src/libfoundation.la $(LIBIPAP_LIBS)
