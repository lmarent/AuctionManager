AUTOMAKE_OPTIONS = foreign

# Rules for the test code (use `make check` to execute)
TESTS = test_runner
check_PROGRAMS = $(TESTS)


test_runner_SOURCES = 	@top_srcdir@/src/Error.cpp \
						@top_srcdir@/src/ParserFcts.cpp \
						@top_srcdir@/src/FieldValue.cpp \
						@top_srcdir@/src/XMLParser.cpp \
						@top_srcdir@/src/Constants.cpp \
						@top_srcdir@/src/ConstantsAum.cpp \
						@top_srcdir@/src/Logger.cpp \
						@top_srcdir@/src/FieldDefParser.cpp \
						@top_srcdir@/src/FieldValParser.cpp \
						@top_srcdir@/src/Bid.cpp \
						@top_srcdir@/src/BidIdSource.cpp \
						@top_srcdir@/src/BidFileParser.cpp \
						@top_srcdir@/test/FieldValue_test.cpp \
						@top_srcdir@/test/FieldDefParser_test.cpp \
						@top_srcdir@/test/FieldValParser_test.cpp \
						@top_srcdir@/test/BidFileParser_test.cpp \
						@top_srcdir@/test/test_runner.cpp

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -g -I@top_srcdir@/include $(CPPUNIT_CFLAGS)
AM_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lcppunit
LDADD = 
