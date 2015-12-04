AUTOMAKE_OPTIONS = foreign

# Rules for the test code (use `make check` to execute)
TESTS = test_runner
check_PROGRAMS = $(TESTS)

test_runner_SOURCES = 	@top_srcdir@/auctioner/src/PageRepository.cpp \
						@top_srcdir@/auctioner/src/CtrlComm.cpp \
						@top_srcdir@/auctioner/src/EventSchedulerAuctioner.cpp \
						@top_srcdir@/auctioner/src/AUMProcessor.cpp \
						@top_srcdir@/auctioner/src/Auctioner.cpp \
						@top_srcdir@/auctioner/test/AUMProcessor_test.cpp \
						@top_srcdir@/auctioner/test/Auctioner_test.cpp \
						@top_srcdir@/auctioner/test/test_runner.cpp

if ENABLE_DEBUG
  AM_CXXFLAGS = -g -I@top_srcdir@/foundation/include $(CPPUNIT_CFLAGS) \
				-I@top_srcdir@/auctioner/include $(LIBIPAP_CFLAGS) \
				$(LIBANSLP_CFLAGS) $(LIBANSLP_MSG_CFLAGS)  \
			    -I$(top_srcdir)/lib/getopt_long -I$(top_srcdir)/lib/httpd \
			    -fno-inline -ggdb -DDEBUG -DPROFILING -DINTEL			    
else
  AM_CXXFLAGS = -O2 -I@top_srcdir@/foundation/include $(CPPUNIT_CFLAGS) \
				-I@top_srcdir@/auctioner/include $(LIBIPAP_CFLAGS) \
				 $(LIBANSLP_CFLAGS) $(LIBANSLP_MSG_CFLAGS) \
			    -I$(top_srcdir)/lib/getopt_long -I$(top_srcdir)/lib/httpd		    
endif
			     
AM_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lcppunit $(top_builddir)/lib/getopt_long/libgetopt_long.a \
			 $(top_builddir)/lib/httpd/libhttpd.a \
			 $(abs_top_builddir)/foundation/src/libfoundation.la
			 	 
LDADD = $(LIBIPAP_LIBS) @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ \
			 $(LIBIPAP_LIBS) $(LIBPROT_LIBS) $(LIBFASTQUEUE_LIBS) \
			 $(LIBGIST_LIBS) $(LIBANSLP_MSG_LIBS) $(LIBANSLP_LIBS) @LIBUUID_LIBS@
