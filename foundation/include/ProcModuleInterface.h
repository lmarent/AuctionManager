
/*! \file  ProcModuleInterface.h

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Quality Manager System (NETQoS).

    NETQoS is free software; you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    NETQoS is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Description:
    Interface definition for QoS policy execution modules

    $Id: ProcModuleInterface.h 748 2015-03-15 16:19:00 amarentes $

*/

#ifndef __PROCMODULEINTERFACE_H
#define __PROCMODULEINTERFACE_H

#include "stdinc.h"
#include "metadata.h"
#include "FieldDefParser.h"
#include "FieldValParser.h"
#include "Field.h"

namespace auction
{

class Auction;
class BiddingObject;


// configuration parameter passed to the module
typedef struct {
    char *name; 
    char *value;
} configParam_t;


typedef int (*timeout_func_t)( int timerID );

//! short   the magic number that will be embedded into every action module
#define PROC_MAGIC   ('N'<<24 | 'M'<<16 | '_'<<8 | 'P')



#define LIST_END       { LISTEND, "LEnd" }
#define EXPORT_END     { EXPORTEND, "EEnd" }


/*! parameter values used for a call to 'getModuleInfo' */
enum ActionInfoNumbers_e {
    /* module function attributes */
    I_MODNAME = 0, 
    I_ID, 
    I_VERSION,
    I_CREATED,
    I_MODIFIED,
    I_BRIEF,
    I_VERBOSE,
    I_HTMLDOCS, /* new */
    I_PARAMS,
    I_RESULTS,
    /* module author attributes */
    I_AUTHOR, 
    I_AFFILI,
    I_EMAIL,
    I_HOMEPAGE,
    I_NUMINFOS
};


typedef struct {
    unsigned int id;
    unsigned int ival_msec;
    unsigned int flags;
} timers_t;


typedef enum {
    TM_NONE = 0, TM_RECURRING = 1, TM_ALIGNED = 2, TM_END = 4
    // , TM_NEXTFLAG = 8, TM_NEXTNEXTFLAG = 16, 32, 64 etc pp.
} timerFlags_e;

#define TIMER_END  { (unsigned int)-1, 0 /*ival==0 marks list end*/, TM_END }


static const int offset_refer[] = {
    ( -14 ),
    ( 0 ),
    ( 0 ),
    ( 0 )
};

//! auction list
typedef vector<Auction*>            auctionDB_t;
typedef vector<Auction*>::iterator  auctionDBIter_t;

//! Bid list
typedef vector<BiddingObject*>             biddingObjectDB_t;
typedef vector<BiddingObject*>::iterator   biddingObjectDBIter_t;

typedef int (*proc_timeout_func_t)( int timerID, void *flowdata );


/*! \short   initialize the action module upon loading 
   \returns 0 - on success, <0 - else 
*/
void initModule( configParam_t *params );


/*! \short   cleanup action module structures before it is unloaded 
   \returns 0 - on success, <0 - else 
*/
void destroyModule( configParam_t *params );


/*! \short   execute the auction for the list of bids given as parameter

    \arg \c  params 			- module parameter text from inside '( )'
    \arg \c  bids   			- bids to include in the execution process.
    \arg \c  allocationdata 	- allocationData returned by the auction process.
    
    Memory allocated within the method and returned in allocationdata should be
    freed by caller. 
*/
void execute( auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals, 
			  configParam_t *params, string aset, string aname, time_t start, time_t stop, biddingObjectDB_t *bids, 
			  biddingObjectDB_t **allocationdata );

/*! \short   execute the bidding process for the list of auctions given 
 * 			 that are required to support a resource request interval.

    \arg \c	 fieldDefs  - definition of fields specified for auctioning. 
    \arg \c  request 	- request object given 
    \arg \c  auctions   - auctions that must be executed. 
    \arg \c  biddata 	- bid created  by the auction process.

    Memory allocated within the method and returned in biddata should be
    freed by caller. 

*/
void execute_user( auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals,
				   fieldList_t *requestparams, auctionDB_t *auctions, biddingObjectDB_t **biddata );


/*! \short   dismantle the module

    attention: do NOT free this slice of memory itself
    \arg \c  Configured parameters given to the module.
    \returns 0 - on success, <0 - else
*/
void destroy( configParam_t *params );


/*! \short   reset flow data record for a rule

    \arg \c  flowdata  - Reset the module to start a new binding process.
    \returns 0 - on success, <0 - else
*/
void reset( configParam_t *params );


/*! \short   provide textual information about this action module

    A string is returned that describes one property (e.g.author) of the
    action module in detail. \n A list of common properties follows in the
    argument list

    \arg \c I_NAME    - name of the action module
    \arg \c I_UID     - return unique module id number (as string)
    \arg \c I_BRIEF   - brief description of the action module functionality
    \arg \c I_AUTHOR  - name/e-mail of the author of this module
    \arg \c I_CREATE  - info about module creation (usually date and similar)
    \arg \c I_DETAIL  - detailed module functionality description
    \arg \c I_PARAM   - description of parameter(s) of module
    \arg \c I_RESULT  - information about nature and format of measurement 
                        results for this module
    \arg \c I_RESERV  - reserved info entry
    \arg \c I_USER    - entry open for free use
    \arg \c I_USER+1  - entry open for free use
    \arg \c I_USER+2  - entry open for free use
    \arg \c I_USER+n  - must return NULL

    \returns - a string which contains textual information about a 
               property of this action module \n
    \returns - pointer to a '\0' string if no information is available \n
    \returns - NULL for index after last stored info string
*/
const char* getModuleInfo( int i );

/*! \short   return error message for last failed function

    \arg \c    - error number (return value from failed function)
    \returns 0 - textual description of error for logging purposes
*/
char* getErrorMsg( int code );



/*! \short   definition of interface struct for Action Modules 

  this structure contains pointers to all functions of this module
  which are part of the Action Module API. It will be automatically 
  set for an Action Module upon compilation (don't forget to include
  ActionModule.h into every module!)
*/

typedef struct {

    int version;

    void (*initModule)( configParam_t *params );
    
    void (*destroyModule)( configParam_t *params );

    void (*execute)( auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals, 
					 configParam_t *params, string aset, string aname, time_t start, 
					 time_t stop, biddingObjectDB_t *bids, biddingObjectDB_t **allocationdata );

	void (*execute_user)( auction::fieldDefList_t *fieldDefs, auction::fieldValList_t *fieldVals,
						  fieldList_t *requestparams, auctionDB_t *auctions, biddingObjectDB_t **biddata );	
					 
    void (*destroy)( configParam_t *params );

    void (*reset)( configParam_t *params );
    
    const char* (*getModuleInfo)(int i);

    char* (*getErrorMsg)( int code );

} ProcModuleInterface_t;

} // namespace auction


#endif /* __PROCMODULEINTERFACE_H */
