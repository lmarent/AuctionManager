
/*! \file   AuctionTimer.h

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Quality Manager System (NETAUM).

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
    Code based on Netmate

    Description:
    Auction timing functions
    Code based on Netmate Implementation

    $Id: AuctionManagerComponent.h 748 2015-07-26 9:55:00 amarentes $
*/

#ifndef _AUCTIONTIMER_H_
#define _AUCTIONTIMER_H_


#include "stdincpp.h"
#include "Logger.h"

namespace auction
{

/*! \short predefined identifiers for auction processes slots */
enum auctSlots 
{ 
    slot1=0, 
    slot2, 
    slot3, 
    MS_MODULE, 
    MS_EXPORT, 
    MS_INSTALL,
    MAXAUCTSLOTS 
};

/*! \short this struct stores a line of measured clock ticks */
struct tickDuration
{
    unsigned long long start; //!< start clock tick counter
    unsigned long long last;  //!< clock tick duration for last measurement
    unsigned long long sum;   //!< accumulated clock ticks
    int num;                  //!< number of measurements
};


//! execution interval definition
typedef struct 
{
    //! execution interval
    unsigned long interval;
    //! align yes/no
    int align;
} interval_t;

//! compare two interval structs
struct lttint
{
    bool operator()(const interval_t i1, const interval_t i2) const
    {
      if  ((i1.interval < i2.interval) ||
           (i1.align < i2.align)) {
          return 1;
      } else {
          return 0;
      }
    }
};


//!   the AuctionTimer class stores timestamps for auction processes

class AuctionTimer
{
  private:

    static AuctionTimer *s_instance;

    Logger *log;
    int ch; //! logging channel number used by objects of this class

    //!< system clock speed in MHz
    double clockSpeed;

    //!< system clock speed in MHz
    unsigned int clockSpeed_int;

    //!< wasted CPU cycles per measurement
    unsigned int overhead;

    //!< table of performance measurement results
    tickDuration ticks[MAXAUCTSLOTS];

    /*! \short   short read system clock speed from /proc/cpuinfo
        \returns system clock speed in MHz, -1.0 if not available
    */
    double readClockSpeed();

    //! construct and initialize a AuctionTimer object
    AuctionTimer();

  public:

    /*! \short   read system clock tick counter from cpu
        \returns number of CPU clock ticks since last reboot
    */
    static __inline__ unsigned long long readTSC()
    {
        unsigned long long x = 0;

#if (INTEL && PROFILING)
        __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
#else
        x = 0;
#endif
        return x;
    }

    //! access the one and only AuctionTimer instance
    static AuctionTimer *getInstance();

    //! destroy a AuctionTimer object
    ~AuctionTimer();

    /*! \short   //!< get system clock speed in MHz
        \returns system clock speed in MHz, -1.0 if not available
    */
    double getClockSpeed() 
    { 
        return clockSpeed; 
    }

    /*! \short   start measurement for one slot
        \arg \c slot - number of measurement slot
    */
    void start( int slot );

    /*! \short   end measurement for one slot and record number of ticks
        \arg \c slot - number of measurement slot
    */
    void stop( int slot );

    /*! \short   account measurement with 'ticks' clock ticks for slot 'slot'
        \arg \c slot - number of measurement slot
        \arg \c ticks - number of CPU clock ticks from independant measurement
    */
    void account( int slot, unsigned long long ticks );

    /*! \short   calculate number of nsec taken for latest measurements for one specific slot
        \arg \c slot - number of measurement slot
    */
    unsigned long long latest( int slot );

    /*! \short   calculate average number of nsec taken for measurements for one slot
        \arg \c slot - number of measurement slot
    */
    unsigned long long avg( int slot );

    /*! \short   convert number of clock ticks into number of nanoseconds
        \arg \c runs  - number of measurements that accumulated the clock ticks
        \arg \c ticks - number of accumulated clock ticks
        \returns number of nanoseconds elapsed during one measurement on average

        warning: this function assumes the measurement overhead is still included
        in the number of clock ticks spent and subtracts runs*overhead !
    */
    static unsigned long long ticks2ns( unsigned int runs, unsigned long long ticks );
    static unsigned long long ticks2ns( unsigned long long ticks );

    //! dump a AuctionTimer object
    void dump( ostream &os );
};


//! overload for <<, so that a AuctionTimer object can be thrown into an ostream
ostream& operator<< ( ostream &os, AuctionTimer &obj );

}; // namespace auction

#endif // _AUCTIONTIMER_H_
