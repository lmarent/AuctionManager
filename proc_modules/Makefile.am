EXTRA_DIST = ProcModule.h ProcModule.cpp ProcError.cpp

lib_LTLIBRARIES = libbas.la

if ENABLE_DEBUG
  AM_CFLAGS = -I$(top_srcdir)/foundation/include \
			  -I/usr/src/linux/include $(LIBIPAP_CFLAGS) \
			  -g -DDEBUG -DPROFILING -D_GLIBCXX_DEBUG
			  
  AM_CXXFLAGS = -I$(top_srcdir)/foundation/include \
			    -I/usr/src/linux/include $(LIBIPAP_CFLAGS) \
			    -g -DDEBUG -DPROFILING -D_GLIBCXX_DEBUG
else
  AM_CFLAGS = -I$(top_srcdir)/foundation/include \
			  -I/usr/src/linux/include $(LIBIPAP_CFLAGS) \
			  -O2
			  
  AM_CXXFLAGS = -I$(top_srcdir)/foundation/include \
			    -I/usr/src/linux/include $(LIBIPAP_CFLAGS) \
			    -O2 
endif

libbas_la_CPPFLAGS = -I$(top_srcdir)/foundation/include \
				  -I/usr/src/linux/include $(LIBIPAP_CFLAGS)
		   
libbas_la_LDFLAGS = -export-dynamic
					
libbas_la_SOURCES = bas.cpp
libbas_la_LIBADD = ProcModule.lo ProcError.lo ../foundation/src/libfoundation.la


