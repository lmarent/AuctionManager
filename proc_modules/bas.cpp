

#include <sys/types.h>
#include <time.h>     
#include <iostream>
#include <stdio.h>
#include "ProcError.h"
#include "ProcModule.h"

void initModule( configParam_t *params )
{

#ifdef DEBUG
	fprintf( stdout, "bas module: start init module \n");
#endif



#ifdef DEBUG
	fprintf( stdout, "bas module: end init module \n");
#endif

}

void destroyModule( configParam_t *params)
{
#ifdef DEBUG
	fprintf( stdout, "bas module: start destroy module \n");
#endif


#ifdef DEBUG
	fprintf( stdout, "bas module: end destroy module \n");
#endif

}

void execute( configParam_t *params, bidDB_t *bids, void **allocationData )
{

#ifdef DEBUG
	fprintf( stdout, "bas module: start execute \n");
#endif

	Bid **bids  


#ifdef DEBUG
	fprintf( stdout, "bas module: end execute \n");
#endif
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

void destroy( configParam_t *params, void *flowdata )
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
