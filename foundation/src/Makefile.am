lib_LTLIBRARIES = libfoundation.la

if ENABLE_DEBUG
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-g  -fPIC -fno-inline -DDEBUG -D_GLIBCXX_DEBUG
else
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-O2  -fPIC 
endif

libfoundation_la_CPPFLAGS = -I$(top_srcdir) \
							-I$(top_srcdir)/foundation/include

libfoundation_la_SOURCES = Logger.cpp \
                          AuctionTimer.cpp \
                          ConfigParser.cpp \
                          ConfigManager.cpp \
                          CommandLineArgs.cpp \
						  Constants.cpp \
						  ConstantsAum.cpp \
						  Error.cpp \
						  FieldValue.cpp \
						  Field.cpp \
						  ParserFcts.cpp \
						  Timeval.cpp \
						  XMLParser.cpp \
						  Allocation.cpp \
						  FieldDefParser.cpp \
						  FieldValParser.cpp \
						  Bid.cpp \
						  BidFileParser.cpp \
						  MAPIBidParser.cpp \
						  AuctionIdSource.cpp \
						  Auction.cpp \
						  AuctionManager.cpp \
						  AuctionFileParser.cpp \
						  MAPIAuctionParser.cpp \
						  BidIdSource.cpp \
						  BidManager.cpp \
						  AuctionManagerInfo.cpp \
						  AuctionManagerComponent.cpp \
						  Event.cpp \
						  EventScheduler.cpp					  
						  

libfoundation_la_LDFLAGS = -export-dynamic -module -shared -fPIC
