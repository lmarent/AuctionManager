
/*!\file   AUMProcessor.cpp

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Auction Manager System (NETAUM).

    NETAUM is free software; you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    NETAUM is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Description:
		manages and applies auction processing modules

    $Id: AUMProcessor.cpp 748 2015-07-23 14:33:00Z amarentes $
*/

#include "ParserFcts.h"
#include "stdinc.h"
#include "ProcError.h"
#include "AUMProcessor.h"
#include "Module.h"
#include "IpAp_create_map.h"


using namespace auction;

setFieldsList_t AUMProcessor::fieldSets;


/* ------------------------- AUMProcessor ------------------------- */

AUMProcessor::AUMProcessor(int domain, ConfigManager *cnf, string fdname, string fvname, int threaded, string moduleDir ) 
    : AuctionManagerComponent(cnf, "AUM_PROCESSOR", threaded), 
	  IpApMessageParser(domain), FieldDefManager(fdname, fvname)
{
    string txt;
    
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

    if (moduleDir.empty()) {
		txt = cnf->getValue("ModuleDir", "AUM_PROCESSOR");
        if ( txt != "") {
            moduleDir = txt;
        }        
    }

    try {
        loader = new ModuleLoader(cnf, moduleDir.c_str() /*module (lib) basedir*/,
                                  cnf->getValue("Modules", "AUM_PROCESSOR"),/*modlist*/
                                  "Proc" /*channel name prefix*/,
                                  getConfigGroup() /* Configuration group */);

#ifdef DEBUG
    log->dlog(ch,"End starting");
#endif

    } catch (Error &e) {
        throw e;
    }
}


/* ------------------------- ~AUMProcessor ------------------------- */

AUMProcessor::~AUMProcessor()
{

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

#ifdef ENABLE_THREADS
    if (threaded) {
        mutexLock(&maccess);
        stop();
        mutexUnlock(&maccess);
        mutexDestroy(&maccess);
    }
#endif
		
    // discard the Module Loader
    saveDelete(loader);

}

miscList_t 
AUMProcessor::readMiscData( ipap_template *templ, ipap_data_record &record)
{
#ifdef DEBUG
    log->dlog(ch, "Starting readMiscData");
#endif

	miscList_t miscs;
	fieldDataListIter_t fieldIter;
	
	for (fieldIter=record.begin(); fieldIter!=record.end(); ++fieldIter)
	{
		ipap_field_key kField = fieldIter->first;
		ipap_value_field dFieldValue = fieldIter->second;
		
		fieldDefItem_t fItem = 
			findField(FieldDefManager::getFieldDefs(), 
						 kField.get_eno() , kField.get_ftype());
			
		if ((fItem.name).empty()){
			ostringstream s;
			s << "AUM Processor: Field eno:" << kField.get_eno();
			s << "fType:" << kField.get_ftype() << "is not parametrized";
			throw Error(s.str()); 
		}
		else
		{
			ipap_field field = templ->get_field( kField.get_eno(), kField.get_ftype() );
			configItem_t item;
			item.name = fItem.name;
			item.type = fItem.type;
			item.value = field.writeValue(dFieldValue);
			miscs[item.name] = item;
		}
	}
		
#ifdef DEBUG
    log->dlog(ch, "Ending readMiscData");
#endif

	return miscs;

}


bool AUMProcessor::intersects( time_t startDttmAuc, time_t stopDttmAuc, 
								 time_t startDttmReq, time_t stopDttmReq)
{

#ifdef DEBUG
	struct timeval t1, t2, t3, t4;
	t1.tv_sec = startDttmAuc;
	t2.tv_sec = stopDttmAuc;
	t3.tv_sec = startDttmReq;
	t4.tv_sec = stopDttmReq;
    log->dlog(ch,"Start intersects %s - %s - %s -%s",  (Timeval::toString(t1)).c_str(), 
									 (Timeval::toString(t2)).c_str(),
									 (Timeval::toString(t3)).c_str(),
									 (Timeval::toString(t4)).c_str() );
#endif

	if (stopDttmReq <= startDttmAuc){		
		return false;
	}
	
	if (stopDttmAuc <= startDttmReq){
		return false;
	}
	
	return true;

}

bool AUMProcessor::forResource(string resourceAuc, string resourceIdReq)
{
#ifdef DEBUG
    log->dlog(ch,"Start forResource resourceAuc:%s, resourceReq:%s", 
					resourceAuc.c_str(), resourceIdReq.c_str());
#endif
	
	if (resourceAuc.compare(resourceIdReq) == 0 ){
		return true;
	}
	
	if (resourceIdReq.compare("any") == 0){
		return true;
	}
	
	return false;
}

/* ----------------------- addAuctionProcess ------------------------- */
int 
AUMProcessor::addAuctionProcess( Auction *a, EventScheduler *evs )
{

#ifdef DEBUG
    log->dlog(ch,"Start addAuctionProcess");
#endif
   
    
    int errNo;
    string errStr;
    bool exThrown = false;

    int auctionId = a->getUId();
    action_t *action = a->getAction();

#ifdef DEBUG
    log->dlog(ch, "Adding auction #%d - set:%s, name:%s", 
				 auctionId, a->getSetName().c_str(), a->getAuctionName().c_str());
#endif  

    AUTOLOCK(threaded, &maccess);  

    Module *mod;
    string mname = action->name;
	auctionProcess entry;

#ifdef DEBUG
    log->dlog(ch, "It is going to load module %s", mname.c_str());
#endif 

    try{        	    
		
		entry.setAuction(a);

		// load Action Module used by this rule
		mod = loader->getModule(mname.c_str());
		entry.setModule( dynamic_cast<ProcModule*> (mod));

		if (entry.getModule() != NULL) { // is it a processing kind of module

#ifdef DEBUG
    log->dlog(ch, "module %s loaded", mname.c_str());
#endif 
			 entry.setProcessModuleInterface( entry.getModule()->getAPI());

			 // init module
			 cout << "Num parameters:"  << (action->conf).size() << endl;
			 entry.setParams( ConfigManager::getParamList( action->conf ));
					
		}
		// success ->enter struct into internal map
		auctions[auctionId] = entry;

    } 
    catch (Error &e) 
    { 
        log->elog(ch, e);
        errNo = e.getErrorNo();
        errStr = e.getError();
		exThrown = true;	
    }
	
	catch (ProcError &e)
	{
        log->elog(ch, e.getError().c_str());
        errNo = e.getErrorNo();
        errStr = e.getError();
		exThrown = true;		
	}

	if (exThrown)
	{
    
        //release packet processing modules already loaded for this rule
        if (entry.getModule()) {
            loader->releaseModule(entry.getModule());
        }

        throw Error(errNo, errStr);;
	}

#ifdef DEBUG
    log->dlog(ch,"End addAuction");
#endif
    
    return auctionId;

}


/*
// delete auctions
void AUMProcessor::delAuctions(auctionDB_t *aucts)
{
    auctionDBIter_t iter;

    for (iter = aucts->begin(); iter != aucts->end(); iter++) {
        delAuction(*iter);
    }
}
*/

/* ------------------------- execute ------------------------- */

void AUMProcessor::executeAuction(int index, time_t start, time_t stop, EventScheduler *e )
{
		
    AUTOLOCK(threaded, &maccess);  

#ifdef DEBUG	
	log->dlog(ch,"Starting executeAuction index:%d start:%s stop:%s", index,
					Timeval::toString(start).c_str(), Timeval::toString(stop).c_str() ); 
#endif	
	
	auctionProcessListIter_t ret;
	ret = auctions.find(index);	
	if (ret != auctions.end()){

		auctionProcess actProcess = auctions[index];
		
		biddingObjectDB_t allocations;
			
		biddingObjectDB_t *ptr = &allocations;
		
		if ( actProcess.getBids()->size() > 0 ){
		
			try {			
				actProcess.getMAPI()->execute( FieldDefManager::getFieldDefs(),
												FieldDefManager::getFieldVals(),
												actProcess.getParams(), 
												actProcess.getAuction()->getSetName(),
												actProcess.getAuction()->getAuctionName(),
												start, stop, 
												actProcess.getBids(), 
												&ptr );

#ifdef DEBUG	
				log->dlog(ch,"Number of allocations generated %d", allocations.size() ); 
#endif	

			} catch (ProcError &e){
				log->elog(ch,e.getError().c_str());
				throw Error(e.getError().c_str());
			}
			
			e->addEvent(new AddGeneratedBiddingObjectsEvent(index, allocations));
		}
		else {
			log->log(ch,"No bids included");
		}
	} else {
		throw Error("auction process with index:%d was not found", index);
	}
}




/* ------------------------- addBiddingObject ------------------------- */

void 
AUMProcessor::addBiddingObjectAuctionProcess( int index, BiddingObject *b )
{

#ifdef DEBUG
    log->dlog(ch, "adding Bidding Object #%d to process auction- %d", b->getUId(), index );
#endif

    AUTOLOCK(threaded, &maccess);

    auctionProcessListIter_t iter = auctions.find(index);
	if (iter != auctions.end() ){  
		if (b->getType() == IPAP_BID){
			(iter->second).insertBid(b);
		}	
    } else {	
		throw Error("process Auction not found: %d", index);
	}
}

void 
AUMProcessor::addBiddingObjectsAuctionProcess( int index, biddingObjectDB_t *bids )
{

	biddingObjectDBIter_t iter;
	for (iter = bids->begin(); iter != bids->end(); ++iter){
		addBiddingObjectAuctionProcess( index, *iter );
	}

}


/* ------------------------- delBiddingObject ------------------------- */

void AUMProcessor::delBiddingObjectAuctionProcess( int index, BiddingObject *b )
{
 
    AUTOLOCK(threaded, &maccess);

    int biddingObjectId = b->getUId();

    
	bool deleted=false;
		
	auctionProcessListIter_t iter = auctions.find(index);
	if (iter != auctions.end()){

#ifdef DEBUG
		log->dlog(ch, "deleting Bidding Object #%d to auction- Set:%s name:%s", biddingObjectId,
					(iter->second).getAuction()->getSetName().c_str(), 
						(iter->second).getAuction()->getAuctionName().c_str());
#endif

				
		biddingObjectDB_t * bidObjects;
		biddingObjectDBIter_t bid_iter;
		
		bidObjects = (iter->second).getBids();

#ifdef DEBUG
		log->dlog(ch, "Nro Bidding Objects:%d", bidObjects->size());
#endif		
		for ( bid_iter = bidObjects->begin();  bid_iter != bidObjects->end(); ++bid_iter ){
			
			if ((b->getBiddingObjectSet().compare((*bid_iter)->getBiddingObjectSet()) == 0) &&
				(b->getBiddingObjectName().compare((*bid_iter)->getBiddingObjectName()) == 0)){
				((iter->second).bids).erase(bid_iter);
				deleted=true;
				break;
			}
		} 
		
	} else { 
		throw Error("Auction process not found: %d", index);
	}
		
	if (deleted==false){
		throw Error("Bid not found: set:%s: name:%s", 
						b->getBiddingObjectSet().c_str(), 
						b->getBiddingObjectName().c_str());
	}
	
}


void AUMProcessor::delBiddingObjectsAuctionProcess(int index, biddingObjectDB_t *bids)
{
	biddingObjectDBIter_t bid_iter;   
	for (bid_iter = bids->begin(); bid_iter!= bids->end(); ++bid_iter ){
		BiddingObject *bid = *bid_iter;
		try {
			 delBiddingObjectAuctionProcess(index, bid );
		} catch(Error &err){
				  log->elog( ch, err.getError().c_str() );
		}
	}
}



/* ------------------------- delAuctionProcess ------------------------- */

void AUMProcessor::delAuctionProcess( int index )
{
    auctionProcess entry;
    
#ifdef DEBUG
    log->dlog(ch, "deleting Auction Process #%d", index);
#endif

    AUTOLOCK(threaded, &maccess);
        
    entry = auctions[index];
            
    // release modules loaded for this rule
    loader->releaseModule(entry.getModule());
     
    auctions.erase(index); 
       
}

/* -----------------  getApplicableAuctions --------------------- */
auctionDB_t * 
AUMProcessor::getApplicableAuctions(ipap_message *message)
{

#ifdef DEBUG
    log->dlog(ch, "start getApplicableAuctions");
#endif

	AUTOLOCK(threaded, &maccess);
	
	auctionDB_t *auctions_anw = new auctionDB_t();
	
	// Read the option data auction template.
	ipap_template *templOptAuct = readTemplate(message, IPAP_OPTNS_AUCTION_TEMPLATE);
		
	// Read the option data record template associated with the option data auction template.
	if (templOptAuct != NULL){
		dataRecordList_t dOptRecordList = readDataRecords(message, templOptAuct->get_template_id());
		
		// If the option template is empty then the returned auction must be zero.		
		if (dOptRecordList.size() > 0){
			dateRecordListIter_t dataIter;
			for (dataIter = dOptRecordList.begin(); dataIter != dOptRecordList.end(); ++dataIter)
			{
				miscList_t miscs = 
					readMiscData( templOptAuct, *dataIter);
				
				fieldDefItem_t field;
				ipap_field ipfield; 
				string sstartDttm = getMiscVal(&miscs, "start"); 
				field = findField(FieldDefManager::getFieldDefs(), "start");
				ipfield = templOptAuct->get_field( field.eno, field.ftype );
				time_t startDttm = (time_t) ipfield.parse(sstartDttm).get_value_int64();
				
				string sstopDttm = getMiscVal(&miscs, "stop");
				field = findField(FieldDefManager::getFieldDefs(), "stop");
				ipfield = templOptAuct->get_field( field.eno, field.ftype );
				time_t stopDttm = (time_t) ipfield.parse(sstopDttm).get_value_int64();
				 
				string resourceId = getMiscVal(&miscs, "resourceid"); 
				
				auctionProcessListIter_t aucIter;
				for (aucIter = auctions.begin(); aucIter != auctions.end(); ++aucIter){
					Auction *auction =  (aucIter->second).auction;
					if (intersects(auction->getStart(), auction->getStop(), startDttm, stopDttm) 
						&& ( forResource(auction->getAuctionResource(), resourceId))){
						auctions_anw->push_back(auction);
					}
				}				
				
			}
		}	
	}

#ifdef DEBUG
    log->dlog(ch, "ending getApplicableAuctions Nbr:%d", auctions_anw->size());
#endif
	
	return auctions_anw;
}

map<ipap_field_key,string> 
AUMProcessor::getSessionInformation(ipap_message *message)
{

#ifdef DEBUG
    log->dlog(ch, "start getSessionInformation");
#endif

	AUTOLOCK(threaded, &maccess);

	map<ipap_field_key, string> sessionInfo;
	
	// Read the option data auction template.
	ipap_template *templOptAuct = readTemplate(message, IPAP_OPTNS_AUCTION_TEMPLATE);
		
	// Read the option data record template associated with the option data auction template.
	if (templOptAuct != NULL){
		dataRecordList_t dOptRecordList = readDataRecords(message, templOptAuct->get_template_id());
		
		// If the option template is empty then the returned auction must be zero.
		// If the option template has more that a record then we assume 
		// that all records have the same session information.
		
		if (dOptRecordList.size() > 0){
			dateRecordListIter_t dataIter;
			for (dataIter = dOptRecordList.begin(); dataIter != dOptRecordList.end(); ++dataIter)
			{
				miscList_t miscs = 
					readMiscData( templOptAuct, *dataIter);
				
				set<ipap_field_key> fields = getSetField(AUM_SESSION_FIELD_SET_NAME);
				set<ipap_field_key>::iterator setIter;
				for (setIter = fields.begin(); setIter != fields.end(); ++setIter){
				
					fieldDefItem_t field = findField(FieldDefManager::getFieldDefs(), 
								setIter->get_eno(), setIter->get_ftype());
					sessionInfo[*setIter] = getMiscVal(&miscs, field.name); 			
				}
				
				break;				
			}
		}	
	}
	
	return sessionInfo;
}


int 
AUMProcessor::handleFDEvent(eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds)
{

    return 0;
}

void 
AUMProcessor::main()
{

    // this function will be run as a single thread inside the AUM processor
    log->log(ch, "AUM Processor thread running");
    
    for (;;) {
        handleFDEvent(NULL, NULL,NULL, NULL);
    }
}       

void 
AUMProcessor::waitUntilDone(void)
{
#ifdef ENABLE_THREADS
    AUTOLOCK(threaded, &maccess);

    if (threaded) {
      while (queue->getUsedBuffers() > 0) {
        threadCondWait(&doneCond, &maccess);
      }
    }
#endif
}


string 
AUMProcessor::getInfo()
{
    ostringstream s;

    AUTOLOCK(threaded, &maccess);

    s << loader->getInfo();  // get the list of loaded modules

    return s.str();
}


/* -------------------- getModuleInfoXML -------------------- */

string AUMProcessor::getModuleInfoXML( string modname )
{
    AUTOLOCK(threaded, &maccess);
    return loader->getModuleInfoXML( modname );
}


/* ------------------------- dump ------------------------- */

void AUMProcessor::dump( ostream &os )
{

    os << "AUM Processor dump :" << endl;
    os << getInfo() << endl;

}


/* ------------------------- operator<< ------------------------- */

ostream& auction::operator<< ( ostream &os, AUMProcessor &pe )
{
    pe.dump(os);
    return os;
}


set<ipap_field_key> 
AUMProcessor::getSetField(agentFieldSet_t setName)
{
	// Fill set of fields.
	if (AUMProcessor::fieldSets.size() == 0 ){
		
		// Fill data auctions fields
		set<ipap_field_key> agentSession;
		agentSession.insert(ipap_field_key(0,IPAP_FT_IPVERSION));
		agentSession.insert(ipap_field_key(0,IPAP_FT_SOURCEIPV4ADDRESS));
		agentSession.insert(ipap_field_key(0,IPAP_FT_SOURCEIPV6ADDRESS));
		agentSession.insert(ipap_field_key(0,IPAP_FT_SOURCEAUCTIONPORT));
		
		
		// Fill option auctions fields
		set<ipap_field_key> auctSearchFields;
		auctSearchFields.insert(ipap_field_key(0,IPAP_FT_STARTSECONDS));
		auctSearchFields.insert(ipap_field_key(0,IPAP_FT_ENDSECONDS));
		auctSearchFields.insert(ipap_field_key(0,IPAP_FT_IDRESOURCE));
				
		AUMProcessor::fieldSets = ipap_create_map<agentFieldSet_t, set<ipap_field_key> >
				(AUM_SESSION_FIELD_SET_NAME,agentSession)
				(AUM_REQUEST_FIELD_SET_NAME,auctSearchFields);
	}
	
	setFieldsListIter_t iter;
	iter = AUMProcessor::fieldSets.find(setName);
	return iter->second;
}
