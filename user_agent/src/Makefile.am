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

netagent_SOURCES =  ResourceRequest.cpp \
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
						  -I$(top_srcdir)/lib/httpd $(LIBIPAP_CFLAGS)

netagent_LDADD = $(top_builddir)/lib/getopt_long/libgetopt_long.a \
					   $(top_builddir)/lib/httpd/libhttpd.a \
					   $(top_builddir)/foundation/src/libfoundation.la \
					   @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -g -I@top_srcdir@/foundation/include 

AM_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lcppunit \
			 $(top_builddir)/lib/getopt_long/libgetopt_long.a \
			 $(top_builddir)/lib/httpd/libhttpd.a \
			 $(top_builddir)/foundation/src/libfoundation.la \
			 @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ $(LIBIPAP_LIBS)
			 
LDADD = $(LIBIPAP_LIBS)
