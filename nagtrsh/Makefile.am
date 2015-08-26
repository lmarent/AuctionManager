if BUILD_NAGTRSH

bin_PROGRAMS = nagtrsh 

if ENABLE_DEBUG
  AM_CFLAGS = 	-I$(top_srcdir)/foundation/include \
				-I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/getpass \
				-g -fno-inline -D_GLIBCXX_DEBUG -DDEBUG -DPROFILING
				
  AM_CXXFLAGS = -I$(top_srcdir)/foundation/include \
				-I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/getpass \
				-g -fno-inline -D_GLIBCXX_DEBUG -DDEBUG -DPROFILING
else
  AM_CFLAGS = -I$(top_srcdir)/foundation/include \
			  -I$(top_srcdir)/lib/getopt_long \
			  -I$(top_srcdir)/lib/getpass \
			  -g -fno-inline -D_GLIBCXX_DEBUG -DDEBUG -DPROFILING -O2

  AM_CXXFLAGS = -I$(top_srcdir)/foundation/include \
				-I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/getpass \
				-g -fno-inline -D_GLIBCXX_DEBUG -DDEBUG -DPROFILING -O2

endif

nagtrsh_SOURCES = nagtrsh.cpp 
				$(top_srcdir)/foundation/src/Error.cpp \
                $(top_srcdir)/foundation/src/CommandLineArgs.cpp \
                $(top_srcdir)/foundation/src/constants.cpp \
                $(top_srcdir)/lib/getopt_long/getopt_long.c \
				$(top_srcdir)/lib/getpass/getpass.c

nagtrsh_LDADD   = @READLINELIB@ @SSLLIB@ @MPATROLLIB@ @CURL_LIBS@ @XMLLIB@ @XSLTLIB@ \
                $(top_builddir)/lib/getopt_long/libgetopt_long.a \
				$(top_builddir)/lib/getpass/libgetpass.a \
				$(top_builddir)/foundation/src/libfoundation.la

$(top_builddir)lib/getopt_long/libgetopt_long.a:
	cd $(top_builddir)/lib/getopt_long ; $(MAKE)

$(top_builddir)/lib/getpass/libgetpass.a:
	cd $(top_builddir)/lib/getpass ; $(MAKE)

else
all:
	@echo "WARNING: not building nqrsh because libxslt and/or libcurl are missing!"
endif
