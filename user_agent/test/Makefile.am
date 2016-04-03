AUTOMAKE_OPTIONS = foreign

# Rules for the test code (use `make check` to execute)
TESTS = test_runner
check_PROGRAMS = $(TESTS)

test_runner_SOURCES = 	@top_srcdir@/user_agent/src/ResourceRequest.cpp \
						@top_srcdir@/user_agent/src/ResourceRequestIdSource.cpp \
						@top_srcdir@/user_agent/src/TextResourceRequestParser.cpp \
						@top_srcdir@/user_agent/src/MAPIResourceRequestParser.cpp \
						@top_srcdir@/user_agent/src/ResourceRequestManager.cpp \
						@top_srcdir@/user_agent/src/ConstantsAgent.cpp \
						@top_srcdir@/user_agent/src/ResourceRequestFileParser.cpp \
						@top_srcdir@/user_agent/src/EventSchedulerAgent.cpp \
						@top_srcdir@/user_agent/src/AgentManagerInfo.cpp \
						@top_srcdir@/user_agent/src/AgentProcessor.cpp \
						@top_srcdir@/user_agent/src/AgentSession.cpp \
						@top_srcdir@/user_agent/src/AgentSessionManager.cpp \
						@top_srcdir@/user_agent/src/PageRepository.cpp \
						@top_srcdir@/user_agent/src/AnslpProcessor.cpp \
						@top_srcdir@/user_agent/src/CtrlComm.cpp \
						@top_srcdir@/user_agent/src/Agent.cpp \
						@top_srcdir@/user_agent/test/ResourceRequest_test.cpp \
						@top_srcdir@/user_agent/test/ResourceRequestFileParser_test.cpp \
						@top_srcdir@/user_agent/test/ResourceRequestManager_test.cpp \
						@top_srcdir@/user_agent/test/AgentProcessor_test.cpp \
						@top_srcdir@/user_agent/test/Agent_test.cpp \
						@top_srcdir@/user_agent/test/AgentThreaded_test.cpp \
						@top_srcdir@/user_agent/test/test_runner.cpp

if ENABLE_DEBUG
  AM_CXXFLAGS = -g -I@top_srcdir@/foundation/include $(CPPUNIT_CFLAGS) \
				-I@top_srcdir@/user_agent/include $(LIBIPAP_CFLAGS) \
				$(LIBANSLP_CFLAGS) $(LIBANSLP_MSG_CFLAGS)  \
			    -I$(top_srcdir)/lib/getopt_long -I$(top_srcdir)/lib/httpd \
			    -fno-inline -ggdb -DDEBUG -DPROFILING -DINTEL
else
  AM_CXXFLAGS = -O2 -I@top_srcdir@/foundation/include $(CPPUNIT_CFLAGS) \
				-I@top_srcdir@/user_agent/include $(LIBIPAP_CFLAGS) \
				 $(LIBANSLP_CFLAGS) $(LIBANSLP_MSG_CFLAGS) \
			    -I$(top_srcdir)/lib/getopt_long -I$(top_srcdir)/lib/httpd
endif
			     
AM_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lcppunit $(top_builddir)/lib/getopt_long/libgetopt_long.a \
			 $(top_builddir)/lib/httpd/libhttpd.a \
			 $(abs_top_builddir)/foundation/src/libauctionfdtion.la
			 
			 
			 	 
LDADD =  $(LIBIPAP_LIBS) @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ \
			 $(LIBIPAP_LIBS) $(LIBPROT_LIBS) $(LIBFASTQUEUE_LIBS) \
			 $(LIBGIST_LIBS) $(LIBANSLP_MSG_LIBS) $(LIBANSLP_LIBS) @LIBUUID_LIBS@ \
			 $(PQXX_LIBS)
