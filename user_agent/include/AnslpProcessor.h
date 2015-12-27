
/*! \file   AnslpProcessor.h

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
		receive and create events for objects returned from the anslp module.

    $Id: AnslpProcessor.h 748 2015-12-23 11:18:00Z amarentes $
*/

#ifndef _ANSLP_PROCESSOR_H_
#define _ANSLP_PROCESSOR_H_


#include "stdincpp.h"
#include "AuctionManagerComponent.h"
#include "Error.h"
#include "Logger.h"
#include "aqueue.h"

namespace auction
{


/*! \short   Manage network events.

    the AgentProcessor class allows and agent to receive 
		and process messages from the network.
*/

class AnslpProcessor : public AuctionManagerComponent
{
  private:
	
	anslp::FastQueue *queue;
	    
  public:

    /*! \short   construct and initialize an AnslpProcessor object

    */
    AnslpProcessor(ConfigManager *cnf, int threaded);

    //!   destroy a Anslp Processor object, to be overloaded
    virtual ~AnslpProcessor();

	anslp::FastQueue *get_fqueue(){ return queue; }

	void process(eventVec_t *e, anslp::AnslpEvent *evt);

    //! handle file descriptor event
    virtual int handleFDEvent(eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds);
	
    //! thread main function
    void main();
      
    //! get information about load module
    string getInfo();

    //! dump a AUMProcessor object
    void dump( ostream &os );

    virtual void waitUntilDone();

};


//! overload for <<, so that a Agent Processor object can be thrown into an iostream
ostream& operator<< ( ostream &os, AnslpProcessor &pe );

}; // namespace auction

#endif // _ANSLP_PROCESSOR_H_
