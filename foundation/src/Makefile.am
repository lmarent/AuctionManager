ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libfoundation.la

if ENABLE_DEBUG
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-g  -fno-inline -DDEBUG -D_GLIBCXX_DEBUG
else
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-O2 
endif

libfoundation_la_CPPFLAGS = -I$(top_srcdir) \
							-I$(top_srcdir)/foundation/include \
							$(LIBGIST_CFLAGS) $(LIBPROT_CFLAGS) $(LIBFASTQUEUE_CFLAGS) \
							$(LIBIPAP_CFLAGS) $(LIBANSLP_MSG_CFLAGS) \
							$(LIBANSLP_CFLAGS)

libfoundation_la_SOURCES = AuctionTimer.cpp \
                          ConfigParser.cpp \
                          ConfigManager.cpp \
                          CommandLineArgs.cpp \
						  Constants.cpp \
						  ConstantsAum.cpp \
						  FieldValue.cpp \
						  Field.cpp \
						  ParserFcts.cpp \
						  Timeval.cpp \
						  XMLParser.cpp \
						  FieldDefParser.cpp \
						  FieldValParser.cpp \
						  MAPIIpApMessageParser.cpp \
						  TemplateIdSource.cpp \
						  AllocationIdSource.cpp \
						  Allocation.cpp \
						  MAPIAllocationParser.cpp \
						  AllocationManager.cpp \
						  BidIdSource.cpp \
						  Bid.cpp \
						  BidFileParser.cpp \
						  MAPIBidParser.cpp \
						  BidManager.cpp \
						  AuctionIdSource.cpp \
						  Auction.cpp \
						  AuctionManager.cpp \
						  AuctionFileParser.cpp \
						  MAPIAuctionParser.cpp \
						  AuctionManagerInfo.cpp \
						  AuctionManagerComponent.cpp \
						  Event.cpp \
						  EventScheduler.cpp \
						  AnslpClient.cpp					  
						  

libfoundation_la_LDFLAGS = -export-dynamic
