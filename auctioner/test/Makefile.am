AUTOMAKE_OPTIONS = foreign

# Rules for the test code (use `make check` to execute)
TESTS = test_runner
check_PROGRAMS = $(TESTS)

test_runner_SOURCES = 	@top_srcdir@/auctioner/src/PageRepository.cpp \
						@top_srcdir@/auctioner/src/CtrlComm.cpp \
						@top_srcdir@/auctioner/src/ProcModule.cpp \
						@top_srcdir@/auctioner/src/Module.cpp \
						@top_srcdir@/auctioner/src/ModuleLoader.cpp \
						@top_srcdir@/auctioner/src/EventSchedulerAuctioner.cpp \
						@top_srcdir@/auctioner/src/AUMProcessor.cpp \
						@top_srcdir@/auctioner/src/Auctioner.cpp \
						@top_srcdir@/auctioner/test/AUMProcessor_test.cpp \
						@top_srcdir@/auctioner/test/test_runner.cpp

if ENABLE_DEBUG
  AM_CXXFLAGS = -g -I@top_srcdir@/foundation/include $(CPPUNIT_CFLAGS) \
				-I@top_srcdir@/auctioner/include \
				$(LIBIPAP_CFLAGS) \
			    -I$(top_srcdir)/lib/getopt_long \
			    -I$(top_srcdir)/lib/httpd \
			    -fno-inline -D_GLIBCXX_DEBUG -DDEBUG -DPROFILING -DINTEL
else
  AM_CXXFLAGS = -O2 -I@top_srcdir@/foundation/include $(CPPUNIT_CFLAGS) \
				-I@top_srcdir@/auctioner/include \
				$(LIBIPAP_CFLAGS) \
			    -I$(top_srcdir)/lib/getopt_long \
			    -I$(top_srcdir)/lib/httpd 
endif
			     
AM_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lcppunit \
			 $(top_builddir)/lib/getopt_long/libgetopt_long.a \
			 $(top_builddir)/lib/httpd/libhttpd.a \
			 @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ \
			 -L$(abs_top_builddir)/foundation/src/libfoundation.la \
			 $(LIBIPAP_LIBS)	
			 	 
LDADD = $(top_builddir)/foundation/src/libfoundation.la	$(LIBIPAP_LIBS)
