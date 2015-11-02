#include <sys/types.h>
#include <time.h>     
#include <iostream>
#include <map>
#include <stdio.h>
#include "config.h"
#include "stdincpp.h"
#include "ProcError.h"
#include "ProcModule.h"

const int MOD_INIT_REQUIRED_PARAMS = 1;

// Variables given as parameters.
double bandwidth_to_sell = 0;
double reserve_price = 0;

double getResourceAvailability( auction::configParam_t *params )
{
 
	 cout << "Starting getResourceAvailability" << endl;
	
     double bandwidth = 0;
     int numparams = 0;
     
     while (params[0].name != NULL) {
		// in all the application we establish the rates and 
		// burst parameters in bytes
				
        if (!strcmp(params[0].name, "Bandwidth")) {
			bandwidth = (double) parseDouble( params[0].value );
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

	cout << "Ending getResourceAvailability - Bandwidth:" << bandwidth << endl;
	
	return bandwidth;
     
}


double getReservePrice( auction::configParam_t *params )
{
     double price = 0;
     int numparams = 0;
     
     cout << "Starting getReservePrice" << endl;
     
     while (params[0].name != NULL) {
		// in all the application we establish the rates and 
		// burst parameters in bytes
				
        if (!strcmp(params[0].name, "ReservePrice")) {
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
	
	cout << "Ending getReservePrice" << price << endl;
		
	return price;
     

}


void auction::initModule( auction::configParam_t *params )
{

	cout <<  "bas module: start init module" << endl;

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

auction::Allocation *
createAllocation( auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals,
				  string auctionSet, string auctionName, string bidSet, string bidName, 
				  double quantity, double price )
{										  		
	auction::fieldList_t fields;
	
	// TODO AM: replace this code with true values
	string allocset = "setalloc";
	string allocname = "allocaname";
		
	auction::field_t field1;
		
	auction::fieldDefListIter_t iter; 
	iter = fieldDefs->find("quantity");
	field1.name = iter->second.name;
	field1.len = iter->second.len;
	field1.type = iter->second.type;
	string fvalue =doubleToString(quantity);
	auction::IpApMessageParser::parseFieldValue(fieldVals, fvalue, &field1);
						
	auction::field_t field2;

	iter = fieldDefs->find("unitprice");
	field2.name = iter->second.name;
	field2.len = iter->second.len;
	field2.type = iter->second.type;
	string fvalue2 = doubleToString(price);
	auction::IpApMessageParser::parseFieldValue(fieldVals, fvalue2, &field2);
		
	fields.push_back(field1);
	fields.push_back(field2);

	auction::allocationIntervalList_t interv;	
	
    auction::Allocation *alloc = new auction::Allocation(auctionSet, auctionName, 
										bidSet, bidName, allocset, allocname,  fields, interv);

	
	return alloc;
}

void incrementQuantityAllocation(auction::Allocation *allocation, double quantity)
{
	auction::fieldList_t *fields = allocation->getFields();
	
	auction::fieldListIter_t field_iter;
	
	for (field_iter = fields->begin(); field_iter != fields->end(); ++field_iter )
	{
		if ((field_iter->name).compare("quantity")){
			auction::field_t field = *field_iter;
			double temp_qty = parseDouble( ((field.value)[0]).getValue());
			temp_qty += quantity;
			string fvalue = doubleToString(temp_qty);
			field_iter->parseFieldValue(fvalue);
			break;
		}
	}
	
}

void auction::execute( auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals,  
					   auction::configParam_t *params, string aset, string aname, auction::bidDB_t *bids, 
					   auction::allocationDB_t **allocationdata )
{

	cout << "bas module: start execute" << (int) bids->size() << endl;

	bandwidth_to_sell = getResourceAvailability(params);
	reserve_price = getReservePrice( params );

	std::multimap<double, alloc_proc_t>  orderedBids;
	// Order Bids by elements.
	auction::bidDBIter_t bid_iter; 
	
	for (bid_iter = bids->begin(); bid_iter != bids->end(); ++bid_iter ){
		auction::Bid * bid = *bid_iter;
				
		auction::elementList_t *elems = bid->getElements();
				
		auction::elementListIter_t elem_iter;
		for ( elem_iter = elems->begin(); elem_iter != elems->end(); ++elem_iter )
		{
			double price = getDoubleField(&(elem_iter->second), "unitprice");
			double quantity = getDoubleField(&(elem_iter->second), "quantity");
			alloc_proc_t alloc;
		
			alloc.bidSet = bid->getBidSet();
			alloc.bidName = bid->getBidName();
			alloc.elementName = elem_iter->first;
			alloc.quantity = quantity;
			orderedBids.insert(make_pair(price,alloc));
		}
	}

	double qtyAvailable = bandwidth_to_sell;
	double sellPrice = 0;
	
	cout << "bas module- qty available:" << qtyAvailable << endl;
	
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

	cout << "bas module: after executing the auction" << (int) bids->size() << endl;
	
	map<string,auction::Allocation *> allocations;
	map<string,auction::Allocation *>::iterator alloc_iter;
	
	// Creates allocations
	it = orderedBids.end();
	do
	{
	    --it;
	    
		if (allocations.find(makeKey(aset, 
			aname,(it->second).bidSet, (it->second).bidName )) != allocations.end()){
			alloc_iter = allocations.find(makeKey(aset, aname,
								(it->second).bidSet, (it->second).bidName ));
			incrementQuantityAllocation(alloc_iter->second, (it->second).quantity); 					
		}
		else{
			auction::Allocation *alloc = 
				createAllocation(fieldDefs, fieldVals, aset, aname, 
								  (it->second).bidSet, (it->second).bidName, 
									(it->second).quantity, sellPrice);
									
			allocations[makeKey(aset, aname,
								(it->second).bidSet, (it->second).bidName)] = alloc;
		}
	    
	} while (it != orderedBids.begin());
	
	// Convert from the map to the final allocationDB result
	auction::allocationDB_t dbResult;
	for ( alloc_iter = allocations.begin(); 
				alloc_iter != allocations.end(); ++alloc_iter )
	{
		(*allocationdata)->push_back(alloc_iter->second);
	}
	
	cout << "bas module: end execute" <<  endl;
}

void auction::execute_user( auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals, 
							auction::fieldList_t *requestparams, auction::auctionDB_t *auctions, 
							auction::bidDB_t **biddata )
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
