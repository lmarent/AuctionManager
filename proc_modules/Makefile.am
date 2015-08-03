EXTRA_DIST = ProcModule.h ProcModule.cpp ProcError.cpp

INCLUDES = -I$(top_srcdir)/include \
				  -I/usr/src/linux/include

lib_LTLIBRARIES = bas.la

if ENABLE_DEBUG
  AM_CFLAGS = -g -DDEBUG -DPROFILING -fPIC @NL3_CFLAGS@ @NL_ROUTE_3_CFLAGS@
  AM_CXXFLAGS = -g -DDEBUG -DPROFILING @NL3_CFLAGS@ @NL_ROUTE_3_CFLAGS@
else
  AM_CFLAGS = -O2 -fPIC @NL3_CFLAGS@ @NL_ROUTE_3_CFLAGS@
  AM_CXXFLAGS = -O2 @NL3_CFLAGS@ @NL_ROUTE_3_CFLAGS@
endif

bas_la_CPPFLAGS = -I$(top_srcdir)/include \
				  -I/usr/src/linux/include
		   
bas_la_LDFLAGS = -export-dynamic -module @NL3_LIBS@ @NL_ROUTE_3_LIBS@
bas_la_SOURCES = bas.cpp
bas_la_LIBADD = ProcModule.lo ProcError.lo


