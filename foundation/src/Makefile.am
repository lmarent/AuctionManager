ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libauctionfdtion.la

INC_DIR 	= $(top_srcdir)/foundation/include

pkginclude_HEADERS = $(INC_DIR)/Auction.h \
					 $(INC_DIR)/AuctioningObject.h \
					 $(INC_DIR)/AuctionManagerInfo.h \
					 $(INC_DIR)/AuctionFileParser.h \
					 $(INC_DIR)/IdSource.h \
					 $(INC_DIR)/BiddingObject.h \
					 $(INC_DIR)/MAPIBiddingObjectParser.h \
					 $(INC_DIR)/BiddingObjectFileParser.h \
					 $(INC_DIR)/BiddingObjectManager.h \
					 $(INC_DIR)/ProcModuleInterface.h \
					 $(INC_DIR)/ProcError.h \
					 $(INC_DIR)/AuctionTimer.h \
					 $(INC_DIR)/Module.h \
					 $(INC_DIR)/ModuleLoader.h \
					 $(INC_DIR)/ProcModule.h \
					 $(INC_DIR)/CommandLineArgs.h \
					 $(INC_DIR)/ConfigManager.h \
					 $(INC_DIR)/Event.h \
					 $(INC_DIR)/EventScheduler.h \
					 $(INC_DIR)/metadata.h \
					 $(INC_DIR)/FieldDefParser.h \
					 $(INC_DIR)/FieldDefManager.h \
					 $(INC_DIR)/FieldValParser.h \
					 $(INC_DIR)/Field.h \
					 $(INC_DIR)/XMLParser.h \
					 $(INC_DIR)/FieldValue.h \
					 $(INC_DIR)/IpApMessageParser.h \
					 $(INC_DIR)/ConfigParser.h \
					 $(INC_DIR)/Constants.h	\
					 $(INC_DIR)/ConstantsAum.h \
					 $(INC_DIR)/Timeval.h \
					 $(INC_DIR)/AnslpClient.h \
					 $(top_srcdir)/lib/getopt_long/getopt_long.h

if ENABLE_DEBUG
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-g  -fno-inline -DDEBUG -ggdb
else
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-O2 
endif

libauctionfdtion_la_CPPFLAGS = -I$(top_srcdir) \
							   -I$(top_srcdir)/foundation/include \
								$(LIBGIST_CFLAGS) $(LIBPROT_CFLAGS) $(LIBFASTQUEUE_CFLAGS) \
								$(LIBIPAP_CFLAGS) $(LIBANSLP_MSG_CFLAGS) \
								$(LIBANSLP_CFLAGS) $(PQXX_CFLAGS)

libauctionfdtion_la_SOURCES = AuctionTimer.cpp \
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
						   IdSource.cpp \
						   FieldDefParser.cpp \
						   FieldValParser.cpp \
						   FieldDefManager.cpp \
						   IpApMessageParser.cpp \
						   AuctioningObject.cpp \
						   TemplateIdSource.cpp \
						   BiddingObject.cpp \
						   BiddingObjectFileParser.cpp \
						   MAPIBiddingObjectParser.cpp \
						   BiddingObjectManager.cpp \
						   Auction.cpp \
						   AuctionManager.cpp \
						   AuctionFileParser.cpp \
						   MAPIAuctionParser.cpp \
						   AuctionManagerInfo.cpp \
						   AuctionManagerComponent.cpp \
						   Session.cpp \
						   SessionManager.cpp \
						   ModuleLoader.cpp \
						   MessageIdSource.cpp \
						   ProcModule.cpp \
						   Module.cpp \
						   Event.cpp \
						   EventScheduler.cpp \
						   AnslpClient.cpp					  
						  

libauctionfdtion_la_LDFLAGS = -export-dynamic
