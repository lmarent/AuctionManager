
/*! \file   AuctionProcessObject.h

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
    manages and applies agent processing functions

    $Id: AuctionProcessObject.h 748 2015-11-03 14:31:00Z amarentes $
*/

#ifndef _AUCTION_PROCESS_OBJECT_H_
#define _AUCTION_PROCESS_OBJECT_H_

namespace auction
{

class AuctionProcessObject
{
	public: 
		
		//! index for uniquely identify this process object.
		int index; 
		
		//! Module loaded to execute the process.
		ProcModule *module; 
		
		//! module API
		ProcModuleInterface_t *mapi; 
	
		AuctionProcessObject(int _index=0, ProcModule *_module=NULL, ProcModuleInterface_t *_mapi=NULL):
			index(_index), module(_module), mapi(_mapi){}
	
		~AuctionProcessObject(){}
		
		void setUId(int _index) { index = _index; }
		
		int getUId(){ return index; }
		
		void setModule(ProcModule *_module ){ module = _module; }
		
		ProcModule *getModule(){ return module; } 
		
		void setProcessModuleInterface(ProcModuleInterface_t * _mapi){ mapi = _mapi; }
		
		ProcModuleInterface_t * getMAPI(){ return mapi; }
	
};

}

#endif // _AUMPROCESSOR_H_
