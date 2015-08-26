#include <sys/types.h>
#include <time.h>     
#include <iostream>
#include <map>
#include <stdio.h>
#include "ProcError.h"
#include "ProcModule.h"

const int MOD_INIT_REQUIRED_PARAMS = 1;

// Variables given as parameters.
double bandwidth_to_sell = 0;
double reserve_price = 0;

double getResourceAvailability( configParam_t *params )
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
		throw ProcError(AUM_PROC_PARAMETER_ERROR, 
					"bas init module - not enought parameters");
	
	if (bandwidth <= 0)
		throw ProcError(AUM_PROC_BANDWIDTH_AVAILABLE_ERROR, 
					"bas init module - The given bandwidth parameter is incorrect");

	cout << "Ending getResourceAvailability - Bandwidth:" << bandwidth << endl;
	
	return bandwidth;
     
}


double getReservePrice( configParam_t *params )
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
		throw ProcError(AUM_PROC_PARAMETER_ERROR, 
					"bas init module - not enought parameters");
	
	if (price < 0)
		throw ProcError(AUM_PRICE_RESERVE_ERROR, 
					"bas init module - The given reserve price is incorrect");
	
	cout << "Ending getReservePrice" << price << endl;
		
	return price;
     

}


void initModule( configParam_t *params )
{

	cout <<  "bas module: start init module" << endl;

	cout << "bas module: end init module" << endl;

}

void destroyModule( configParam_t *params )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start destroy module \n");
#endif


#ifdef DEBUG
	fprintf( stdout, "bas module: end destroy module \n");
#endif

}

double getDoubleField(fieldList_t *fields, string name)
{
	cout << "starting getDoubleField" << name << "num fields:" << fields->size() << endl;
	
	fieldListIter_t field_iter;
		
	for (field_iter = fields->begin(); field_iter != fields->end(); ++field_iter )
	{
	
		if ((field_iter->name).compare(name) == 0 ){
			return parseDouble( ((field_iter->value)[0]).getValue());
		}
	}
	
	throw ProcError(AUM_FIELD_NOT_FOUND_ERROR, 
					"bas init module - The given field was not included");
}

string double_to_string (double value)
{
	std::ostringstream s;
	s << value;
	return s.str();
}

string makeKey(string auctionSet, string auctionName, 
				  string bidSet, string bidName)
{
	return auctionSet + auctionName + bidSet + bidName;
}

Allocation *createAllocation(string auctionSet, string auctionName, 
							  string bidSet, string bidName, 
							  fieldDefList_t *fieldDefs, 
							  double quantity, double price)
{										  		
	fieldList_t fields;
		
	field_t field1;
		
	fieldDefListIter_t iter; 
	iter = fieldDefs->find("quantity");
	field1.name = iter->second.name;
	field1.len = iter->second.len;
	field1.type = iter->second.type;
	string fvalue =double_to_string(quantity);
	field1.parseFieldValue(fvalue);
						
	field_t field2;

	iter = fieldDefs->find("unitprice");
	field2.name = iter->second.name;
	field2.len = iter->second.len;
	field2.type = iter->second.type;
	string fvalue2 = double_to_string(price);
	field2.parseFieldValue(fvalue2);
		
	fields.push_back(field1);
	fields.push_back(field2);

	allocationIntervalList_t interv;	
	
    Allocation *alloc = new Allocation(auctionSet, auctionName, 
										bidSet, bidName, fields, interv);

	
	return alloc;
}

void incrementQuantityAllocation(Allocation *allocation, double quantity)
{
	fieldList_t *fields = allocation->getFields();
	
	fieldListIter_t field_iter;
	
	for (field_iter = fields->begin(); field_iter != fields->end(); ++field_iter )
	{
		if ((field_iter->name).compare("quantity")){
			field_t field = *field_iter;
			double temp_qty = parseDouble( ((field.value)[0]).getValue());
			temp_qty += quantity;
			string fvalue = double_to_string(temp_qty);
			field_iter->parseFieldValue(fvalue);
			break;
		}
	}
	
}

void execute( configParam_t *params, string aset, string aname, 
			  fieldDefList_t *fieldDefs,  bidDB_t *bids, 
			   allocationDB_t **allocationdata )
{

	cout << "bas module: start execute" << (int) bids->size() << endl;

	bandwidth_to_sell = getResourceAvailability(params);
	reserve_price = getReservePrice( params );

	std::multimap<double, alloc_proc_t>  orderedBids;
	// Order Bids by elements.
	bidDBIter_t bid_iter; 
	
	for (bid_iter = bids->begin(); bid_iter != bids->end(); ++bid_iter ){
		Bid * bid = *bid_iter;
				
		elementList_t *elems = bid->getElements();
				
		elementListIter_t elem_iter;
		for ( elem_iter = elems->begin(); elem_iter != elems->end(); ++elem_iter )
		{
			double price = getDoubleField(&(elem_iter->fields), "unitprice");
			double quantity = getDoubleField(&(elem_iter->fields), "quantity");
			alloc_proc_t alloc;
		
			alloc.bidSet = bid->getSetName();
			alloc.bidName = bid->getBidName();
			alloc.elementName = elem_iter->name;
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
	
	map<string,Allocation *> allocations;
	map<string,Allocation *>::iterator alloc_iter;
	
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
			Allocation *alloc = createAllocation(aset, aname, 
												   (it->second).bidSet, (it->second).bidName, 
													fieldDefs, 
													(it->second).quantity, sellPrice);
			allocations[makeKey(aset, aname,
								(it->second).bidSet, (it->second).bidName)] = alloc;
		}
	    
	} while (it != orderedBids.begin());
	
	// Convert from the map to the final allocationDB result
	allocationDB_t dbResult;
	for ( alloc_iter = allocations.begin(); 
				alloc_iter != allocations.end(); ++alloc_iter )
	{
		(*allocationdata)->push_back(alloc_iter->second);
	}
	
	cout << "bas module: end execute" <<  endl;
}

timers_t* getTimers( )
{

#ifdef DEBUG
	fprintf( stdout, "bas module: start getTimers \n");
#endif

	return NULL;

#ifdef DEBUG
	fprintf( stdout, "bas module: end getTimers \n");
#endif

}

void destroy( configParam_t *params, allocationDB_t *allocationdata )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start destroy \n");
#endif

#ifdef DEBUG
	fprintf( stdout, "bas module: end destroy \n");
#endif
}

void reset( configParam_t *params )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start reset \n");
#endif

#ifdef DEBUG
	fprintf( stdout, "bas module: end reset \n");
#endif
}

int timeout( int timerID )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start timeout \n");
#endif
	return 0;
#ifdef DEBUG
	fprintf( stdout, "bas module: end timeout \n");
#endif
}

const char* getModuleInfo( int i )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start getModuleInfo \n");
#endif

    /* fprintf( stderr, "count : getModuleInfo(%d)\n",i ); */

    switch(i) {
    case I_MODNAME:    return "Basic Auction procedure";
    case I_ID:		   return "bas";
    case I_VERSION:    return "0.1";
    case I_CREATED:    return "2015/08/03";
    case I_MODIFIED:   return "2015/08/03";
    case I_BRIEF:      return "Auction process to verify general functionality of the auction manager";
    case I_VERBOSE:    return "The auction process gives does not care about capacity and gives allocations equal to quantity requested for all bids"; 
    case I_HTMLDOCS:   return "http://www.uniandes.edu.co/... ";
    case I_PARAMS:     return "None";
    case I_RESULTS:    return "The set of assigments";
    case I_AUTHOR:     return "Andres Marentes";
    case I_AFFILI:     return "Universidad de los Andes, Colombia";
    case I_EMAIL:      return "la.marentes455@uniandes.edu.co";
    case I_HOMEPAGE:   return "http://homepage";
    default: return NULL;
    }

#ifdef DEBUG
	fprintf( stdout, "bas module: end getModuleInfo \n");
#endif
}

char* getErrorMsg( int code )
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start getErrorMsg \n");
#endif

#ifdef DEBUG
	fprintf( stdout, "bas module: end getErrorMsg \n");
#endif
}
