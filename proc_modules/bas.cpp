#include <sys/types.h>
#include <time.h>     
#include <iostream>
#include <map>
#include <stdio.h>
#include <inttypes.h>
#include "config.h"
#include "stdincpp.h"
#include "ProcError.h"
#include "ProcModule.h"

const int MOD_INIT_REQUIRED_PARAMS = 1;

// Variables given as parameters.
float bandwidth_to_sell = 0;
double reserve_price = 0;
uint32_t lastId;
ipap_field_container g_ipap_fields;

float getResourceAvailability( auction::configParam_t *params )
{
 
#ifdef DEBUG
	 cout << "Starting getResourceAvailability" << endl;
#endif
	
     float bandwidth = 0;
     int numparams = 0;
     
     while (params[0].name != NULL) {
		// in all the application we establish the rates and 
		// burst parameters in bytes
				        
        if (!strcmp(params[0].name, "bandwidth")) {
			bandwidth = (float) parseFloat( params[0].value );
			numparams++;
		}
        params++;
     }
     
     if (numparams == 0)
		throw auction::ProcError(AUM_PROC_PARAMETER_ERROR, 
					"bas init module - not enought parameters");
	
	if (bandwidth <= 0)
		throw auction::ProcError(AUM_PROC_BANDWIDTH_AVAILABLE_ERROR, 
					"bas init module - The given bandwidth parameter is incorrect");

#ifdef DEBUG
	cout << "Ending getResourceAvailability - Bandwidth:" << bandwidth << endl;
#endif
	
	return bandwidth;
     
}


double getReservePrice( auction::configParam_t *params )
{
     double price = 0;
     int numparams = 0;
     
#ifdef DEBUG
     cout << "Starting getReservePrice" << endl;
#endif
     
     while (params[0].name != NULL) {
		// in all the application we establish the rates and 
		// burst parameters in bytes
				
        if (!strcmp(params[0].name, "reserveprice")) {
			price = (double) parseDouble( params[0].value );
			numparams++;
		}
        params++;
     }
     
     if (numparams == 0)
		throw auction::ProcError(AUM_PROC_PARAMETER_ERROR, 
					"bas init module - not enought parameters");
	
	if (price < 0)
		throw auction::ProcError(AUM_PRICE_RESERVE_ERROR, 
					"bas init module - The given reserve price is incorrect");
	
#ifdef DEBUG
	cout << "Ending getReservePrice" << price << endl;
#endif
		
	return price;
     

}

time_t getTime( auction::configParam_t *params, string name )
{
     time_t tim = 0;
     int numparams = 0;
     
#ifdef DEBUG
     cout << "get time - field:" << name << endl;
#endif
     
     while (params[0].name != NULL) {
						
        if (!strcmp(params[0].name, name.c_str())) {
			tim = (time_t) parseTime(params[0].value); 
			numparams++;
		}
        params++;
     }
     
     if (numparams == 0)
		throw auction::ProcError(AUM_PROC_PARAMETER_ERROR, 
					"bas init module - not enought parameters");
	
	if (tim == 0)
		throw auction::ProcError(AUM_DATETIME_NOT_DEFINED_ERROR, 
					"bas init module - The given time is incorrect");
	
#ifdef DEBUG
	cout << "get time" << tim << endl;
#endif
		
	return tim;
     
}


void auction::initModule( auction::configParam_t *params )
{

	cout <<  "bas module: start init module" << endl;

	// Bring fields defined for ipap_messages;
	g_ipap_fields.initialize_forward();
    g_ipap_fields.initialize_reverse();
	
	cout << "bas module: end init module" << endl;

}

void auction::destroyModule( auction::configParam_t *params )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start destroy module \n");
#endif


#ifdef DEBUG
	fprintf( stdout, "bas module: end destroy module \n");
#endif

}


string makeKey(string auctionSet, string auctionName, 
				  string bidSet, string bidName)
{
	return auctionSet + auctionName + bidSet + bidName;
}

auction::BiddingObject *
createAllocation( auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals,
				  string auctionSet, string auctionName, string bidSet, string bidName, 
				  string sessionId, time_t start, time_t stop, float quantity, double price )
{										  		
#ifdef DEBUG
	fprintf( stdout, "bas module: start create allocation \n");
#endif

	uint64_t timeUint64;
	
	auction::elementList_t elements;
    auction::optionList_t options;

	auction::fieldList_t elementFields;
	string elementName = "key_1";
	
	// The set for the allocation is the same as the initial bid.
	string allocset = bidSet;
	
	// Incrememt the next id given.
	uint32_t lid = getId();
	string allocname = uint32ToString(lid);
		
	// Insert quantity
	ipap_field fQuantity = g_ipap_fields.get_field(0, IPAP_FT_QUANTITY);	
	ipap_value_field fVQuantity = fQuantity.get_ipap_value_field( quantity );
	string squantity = fQuantity.writeValue(fVQuantity);
	fillField(fieldDefs, fieldVals, 0, IPAP_FT_QUANTITY, squantity, &elementFields);

#ifdef DEBUG
	fprintf( stdout, "bas module: start create allocation after quantity \n");
#endif

	// Insert Unit Value
	ipap_field fvalue = g_ipap_fields.get_field(0, IPAP_FT_UNITVALUE);	
	ipap_value_field fVValue = fvalue.get_ipap_value_field( price );
	string svalue = fvalue.writeValue(fVValue);
	fillField(fieldDefs, fieldVals, 0, IPAP_FT_UNITVALUE, svalue, &elementFields);

#ifdef DEBUG
	fprintf( stdout, "bas module: start create allocation after unitvalue \n");
#endif

	string recordId = "Unique";
	fillField(fieldDefs, fieldVals, 0, IPAP_FT_IDRECORD, recordId, &elementFields);
		
	elements[elementName] = elementFields;
	
	// construct the interval with the allocation, based on start datetime 
	// and interval for the requesting auction

	auction::fieldList_t optionFields;

	// Insert start
	ipap_field fStart = g_ipap_fields.get_field(0, IPAP_FT_STARTSECONDS);	
	timeUint64 = *reinterpret_cast<uint64_t*>(&start);
	ipap_value_field fVStart = fStart.get_ipap_value_field( timeUint64 );
	string sstart = fStart.writeValue(fVStart);
	fillField(fieldDefs, fieldVals, 0, IPAP_FT_STARTSECONDS, sstart, &optionFields);

#ifdef DEBUG
	fprintf( stdout, "bas module: start create allocation after startSeconds \n");
#endif
	
	// Insert stop
	ipap_field fStop = g_ipap_fields.get_field(0, IPAP_FT_ENDSECONDS);
	timeUint64 = *reinterpret_cast<uint64_t*>(&stop);	
	ipap_value_field fVStop = fStop.get_ipap_value_field( timeUint64 );
	string sstop = fStop.writeValue(fVStop);
	fillField(fieldDefs, fieldVals, 0, IPAP_FT_ENDSECONDS, sstop, &optionFields);

#ifdef DEBUG
	fprintf( stdout, "bas module: start create allocation after endSeconds \n");
#endif		

	fillField(fieldDefs, fieldVals, 0, IPAP_FT_IDRECORD, recordId, &optionFields);
		
	options.push_back(pair<string, auction::fieldList_t>(elementName, optionFields));
	
    auction::BiddingObject *alloc = new auction::BiddingObject(auctionSet, auctionName, 
										allocset, allocname, IPAP_ALLOCATION, elements, options);

	// All objects must be inherit the session from the bid.
	alloc->setSession(sessionId);
	
#ifdef DEBUG
	fprintf( stdout, "bas module: ending create allocation \n");
#endif	

	return alloc;
}

void incrementQuantityAllocation(auction::fieldValList_t *fieldVals, 
									auction::BiddingObject *allocation, float quantity)
{

#ifdef DEBUG
	fprintf( stdout, "bas module: starting increment quantity allocation \n");
#endif	


	auction::elementList_t *elements = allocation->getElements();
	
	// there is only one element. 
	auction::fieldListIter_t field_iter;
	auction::field_t field;
	for (field_iter = (elements->begin()->second).begin(); 
				field_iter != (elements->begin()->second).end(); ++field_iter )
	{
		if ((field_iter->name).compare("quantity")){
			// Delete the field. 
			field = *field_iter; 
			(elements->begin()->second).erase(field_iter);
			break;
		}
	}
	
	if ( !(field.name.empty())){
		// Insert again the field.
		float temp_qty = parseFloat( ((field.value)[0]).getValue());
		temp_qty += quantity;
		string fvalue = floatToString(temp_qty);
		auction::IpApMessageParser::parseFieldValue(fieldVals, fvalue, &field);
		(elements->begin()->second).push_back(field);
	} else {
		throw auction::ProcError("Field quantity was not included in the allocation");
	}

#ifdef DEBUG
	fprintf( stdout, "bas module: ending increment quantity allocation \n");
#endif	
	
}

void auction::execute( auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals,  
					   auction::configParam_t *params, string aset, string aname, time_t start, 
					   time_t stop, auction::biddingObjectDB_t *bids, 
					   auction::biddingObjectDB_t **allocationdata )
{

#ifdef DEBUG
	cout << "bas module: start execute" << (int) bids->size() << endl;
#endif

	bandwidth_to_sell = getResourceAvailability(params);
	reserve_price = getReservePrice( params );	

	std::multimap<double, alloc_proc_t>  orderedBids;
	// Order Bids by elements.
	auction::biddingObjectDBIter_t bid_iter; 
	
	for (bid_iter = bids->begin(); bid_iter != bids->end(); ++bid_iter){
		auction::BiddingObject * bid = *bid_iter;
				
		auction::elementList_t *elems = bid->getElements();
				
		auction::elementListIter_t elem_iter;
		for ( elem_iter = elems->begin(); elem_iter != elems->end(); ++elem_iter )
		{
			double price = getDoubleField(&(elem_iter->second), "unitprice");
			double quantity = getDoubleField(&(elem_iter->second), "quantity");
			
			alloc_proc_t alloc;
		
			alloc.bidSet = bid->getBiddingObjectSet();
			alloc.bidName = bid->getBiddingObjectName();
			alloc.elementName = elem_iter->first;
			alloc.sessionId = bid->getSession();
			alloc.quantity = quantity;
			orderedBids.insert(make_pair(price,alloc));
		}
	}

	float qtyAvailable = bandwidth_to_sell;
	double sellPrice = 0;

#ifdef DEBUG	
	cout << "bas module- qty available:" << qtyAvailable << endl;
#endif
	
	std::multimap<double, alloc_proc_t>::iterator it = orderedBids.end();
	do
	{ 
	    --it;
                
        if ( qtyAvailable < (it->second).quantity){
			(it->second).quantity = qtyAvailable;
			if (qtyAvailable > 0){
				sellPrice = it->first; 
				qtyAvailable = 0;
			 }
		}
		else{
			qtyAvailable = qtyAvailable - (it->second).quantity;
			sellPrice = it->first;
		}
		
	} while (it != orderedBids.begin());
	
	// There are more units available than requested 
	if ( qtyAvailable > 0 ){
		sellPrice = reserve_price;
	}
	
	// Selling price is less than reserve price, so let reserve price.
	if (sellPrice < reserve_price){
		sellPrice = reserve_price;
	} 
	
#ifdef DEBUG	
	cout << "bas module: after executing the auction" << (int) bids->size() << endl;
#endif
	
	map<string,auction::BiddingObject *> allocations;
	map<string,auction::BiddingObject *>::iterator alloc_iter;
	
	// Creates allocations
	it = orderedBids.end();
	do
	{
	    --it;
	    
		if (allocations.find(makeKey(aset, 
			aname,(it->second).bidSet, (it->second).bidName )) != allocations.end()){
			alloc_iter = allocations.find(makeKey(aset, aname,
								(it->second).bidSet, (it->second).bidName ));
			incrementQuantityAllocation(fieldVals, alloc_iter->second, (it->second).quantity); 					
		}
		else{
			auction::BiddingObject *alloc = 
				createAllocation(fieldDefs, fieldVals, aset, aname, 
								  (it->second).bidSet, (it->second).bidName, (it->second).sessionId,
								    start, stop, (it->second).quantity, sellPrice);
									
			allocations[makeKey(aset, aname,
								(it->second).bidSet, (it->second).bidName)] = alloc;
		}
	    
	} while (it != orderedBids.begin());
	
	// Convert from the map to the final allocationDB result
	for ( alloc_iter = allocations.begin(); 
				alloc_iter != allocations.end(); ++alloc_iter )
	{
		(*allocationdata)->push_back(alloc_iter->second);
	}
	
#ifdef DEBUG	
	cout << "bas module: end execute" <<  endl;
#endif
}

void auction::execute_user( auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals, 
							auction::fieldList_t *requestparams, auction::auctionDB_t *auctions, 
							time_t start, time_t stop, auction::biddingObjectDB_t **biddata )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start execute_user \n");
#endif
	// Nothing to do

#ifdef DEBUG
	fprintf( stdout, "bas module: end execute_user \n");
#endif
	
}

void auction::destroy( auction::configParam_t *params )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start destroy \n");
#endif

#ifdef DEBUG
	fprintf( stdout, "bas module: end destroy \n");
#endif
}

void auction::reset( auction::configParam_t *params )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start reset \n");
#endif

#ifdef DEBUG
	fprintf( stdout, "bas module: end reset \n");
#endif
}

const char* auction::getModuleInfo( int i )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start getModuleInfo \n");
#endif

    /* fprintf( stderr, "count : getModuleInfo(%d)\n",i ); */

    switch(i) {
    case auction::I_MODNAME:    return "Basic Auction procedure";
    case auction::I_ID:		   return "bas";
    case auction::I_VERSION:    return "0.1";
    case auction::I_CREATED:    return "2015/08/03";
    case auction::I_MODIFIED:   return "2015/08/03";
    case auction::I_BRIEF:      return "Auction process to verify general functionality of the auction manager";
    case auction::I_VERBOSE:    return "The auction process gives does not care about capacity and gives allocations equal to quantity requested for all bids"; 
    case auction::I_HTMLDOCS:   return "http://www.uniandes.edu.co/... ";
    case auction::I_PARAMS:     return "None";
    case auction::I_RESULTS:    return "The set of assigments";
    case auction::I_AUTHOR:     return "Andres Marentes";
    case auction::I_AFFILI:     return "Universidad de los Andes, Colombia";
    case auction::I_EMAIL:      return "la.marentes455@uniandes.edu.co";
    case auction::I_HOMEPAGE:   return "http://homepage";
    default: return NULL;
    }

#ifdef DEBUG
	fprintf( stdout, "bas module: end getModuleInfo \n");
#endif
}

char* auction::getErrorMsg( int code )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start getErrorMsg \n");
#endif
	
	return NULL;

#ifdef DEBUG
	fprintf( stdout, "bas module: end getErrorMsg \n");
#endif
}
