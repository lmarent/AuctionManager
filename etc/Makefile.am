## Makefile.am -- Process this file with automake to produce Makefile.in

dist_sysconf_DATA = auctionfile.dtd bidfile.dtd fielddef.xml \
                    fielddef.dtd fieldval.xml fieldval.dtd netaum.conf.dtd \
                    netagnt.conf.dtd  reply.xml reply.xsl reply2.xsl \
					example_auctions1.xml example_auctions2.xml \
					main.html main_agnt.html example_resource_request1.xml \
					resourcerequestfile.dtd
		    
nodist_sysconf_DATA = netaum.conf.xml netagnt.conf.xml 

DISTCLEANFILES = gen_cert.sh netaum.conf.xml netagnt.conf.xml

if ENABLE_SSL
  nodist_bin_SCRIPTS = gen_cert.sh 
  DISTCLEANFILES += netaum.pem
endif

install-exec-hook: 
if ENABLE_SSL
	@echo "Generating certificate for SSL" ; \
	$(DESTDIR)/$(bindir)/gen_cert.sh $(DESTDIR)/$(sysconfdir)
endif
