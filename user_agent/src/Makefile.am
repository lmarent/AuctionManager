AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = netagent

if ENABLE_DEBUG
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-I$(top_srcdir)/foundation/include \
				-g -fno-inline -ggdb -DDEBUG -DPROFILING -std=c++11
else
  AM_CXXFLAGS = -O2  
				-I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-I$(top_srcdir)/foundation/include -std=c++11
endif

netagent_SOURCES =  ResourceRequest.cpp \
					ResourceRequestIdSource.cpp \
					ResourceRequestFileParser.cpp \
					MAPIResourceRequestParser.cpp \
					TextResourceRequestParser.cpp \
					ResourceRequestManager.cpp \
					AgentSession.cpp \
					EventSchedulerAgent.cpp \
					PageRepository.cpp \
					CtrlComm.cpp \
					AgentManagerInfo.cpp \
					AgentSessionManager.cpp \
					AgentProcessor.cpp \
					ConstantsAgent.cpp \
					Agent.cpp \
					main.cpp

netagent_CPPFLAGS = -I$(top_srcdir)/lib/getopt_long \
					-I$(top_srcdir)/lib/httpd \
					$(LIBGIST_CFLAGS) $(LIBPROT_CFLAGS) $(LIBFASTQUEUE_CFLAGS) \
					$(LIBIPAP_CFLAGS) $(LIBANSLP_MSG_CFLAGS) $(LIBANSLP_CFLAGS) \
					@LIBUUID_CFLAGS@ -I@top_srcdir@/foundation/include $(PQXX_CFLAGS) \
					-I$(top_srcdir)/user_agent/include

netagent_LDADD =  $(top_builddir)/lib/getopt_long/libgetopt_long.a \
				 $(top_builddir)/lib/httpd/libhttpd.a
				 $(abs_top_builddir)/foundation/src/libauctionfdtion.la \
				 @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ @LIBUUID_LIBS@  $(PQXX_LIBS)

netagent_LDFLAGS =  -export-dynamic -module -std=c++11 \
					$(CPPUNIT_LIBS) -ldl -lcppunit $(top_builddir)/lib/getopt_long/libgetopt_long.a \
					$(top_builddir)/lib/httpd/libhttpd.a \
					$(abs_top_builddir)/foundation/src/libauctionfdtion.la \
					@PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ @LIBUUID_LIBS@ $(LIBIPAP_LIBS) \
					$(LIBPROT_LIBS) $(LIBFASTQUEUE_LIBS) $(LIBGIST_LIBS) \
					$(LIBANSLP_MSG_LIBS) $(LIBANSLP_LIBS) $(PQXX_LIBS)


# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -g  -I@top_srcdir@/include

			 
LDADD = $(LIBIPAP_LIBS)

$(top_builddir)/lib/httpd/libhttpd.a:
	cd $(top_builddir)/lib/httpd ; $(MAKE)

$(top_builddir)/lib/getopt_long/libgetopt_long.a:
	cd $(top_builddir)/lib/getopt_long ; $(MAKE)

$(top_builddir)/foundation/src/libauctionfdtion.la:
	cd $(top_builddir)/foundation/src ; $(MAKE)

install-data-hook:
	$(INSTALL) -d $(DESTDIR)/$(localstatedir)/run
	$(INSTALL) -d $(DESTDIR)/$(localstatedir)/log
