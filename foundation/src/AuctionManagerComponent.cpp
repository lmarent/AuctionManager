
/*!  \file   AuctionManagerComponent.cpp

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Auction Manager System (NETAUM).

    NETAUM is free software; you can redistribute it and/or modify 
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
		Auction component base class, provides common functionality
		for all auction components (e.g. threads, fd handling, ...)

    $Id: AuctionManagerComponent.cpp 748 2015-03-05 18:25:00 amarentes $
*/

#include "ParserFcts.h"
#include "AuctionManagerComponent.h"

using namespace auction;

AuctionManagerComponent::AuctionManagerComponent(ConfigManager *_cnf, string name, int thread )
    :   running(0), cname(name), threaded(thread),  cnf(_cnf)
{
    log  = Logger::getInstance();
    ch   = log->createChannel( name );
    perf = AuctionTimer::getInstance();

#ifdef ENABLE_THREADS
    if (threaded) {
        mutexInit(&maccess);
		threadCondInit(&doneCond);
    }
#endif

}


AuctionManagerComponent::~AuctionManagerComponent()
{
#ifdef ENABLE_THREADS
    if (threaded) {
        mutexLock(&maccess);
        stop();
        mutexUnlock(&maccess);
        mutexDestroy(&maccess);
		threadCondDestroy(&doneCond);
    }
#endif
}


void AuctionManagerComponent::run(void)
{
#ifdef ENABLE_THREADS
    if (threaded && !running) {
		int res = threadCreate(&thread, thread_func, this);
		if (res != 0) {
			throw Error("Cannot create thread within %s: %s",
				cname.c_str(), strerror(res));
		}
		running = 1;
    }
#endif
}


void AuctionManagerComponent::stop(void)
{
#ifdef ENABLE_THREADS
    if (threaded && running) {
		threadCancel(thread);
		threadJoin(thread);
		running = 0;
    }
#endif
}


void* AuctionManagerComponent::thread_func(void *arg)
{
#ifdef ENABLE_THREADS
    // asynch cancel
    threadSetCancelType(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    ((AuctionManagerComponent *)arg)->main();
#endif
    return NULL;
}


void AuctionManagerComponent::main(void)
{
    // nothing here
}


