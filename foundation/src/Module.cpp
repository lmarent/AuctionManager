
/*!\file   Module.cc

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
    this class represents the base class for
    auction processing modules 

    $Id: Module.cpp 748 2015-07-31 14:23:00Z amarentes $
*/

#include "Error.h"
#include "Logger.h"
#include "Module.h"

using namespace auction;

/* ------------------------- Module ------------------------- */

Module::Module( string name, string libfile, libHandle_t libhandle )
{
    log = Logger::getInstance();
    ch = log->createChannel("Module");
#ifdef DEBUG
    log->dlog(ch, "Creating");
#endif

    modName = name;
    fileName = libfile;
    libHandle = libhandle;
    ownName = "";
    refs = 0;
    calls = 0;
}


/* ------------------------- ~Module ------------------------- */

Module::~Module()
{
    dlclose(libHandle);
    
#ifdef DEBUG
    log->dlog(ch, "Destroyed");
#endif
}


/* ------------------------- loadAPI ------------------------- */

void *Module::loadAPI( string apiName )
{
    void *funcList;

    funcList = dlsym(libHandle, apiName.c_str());
    if (funcList == NULL) {
        throw Error("cannot find API called '%s' in lib %s, error: %s",
                    apiName.c_str(), fileName.c_str(), dlerror());
    } else {
        return funcList;
    }
}


/* ------------------------- checkMagic ------------------------- */

void Module::checkMagic( int magicNumber )
{
    // test for magic number in loaded module
    int *magic = (int *)dlsym(libHandle, "magic");

    if (magic == NULL) {
        throw Error("invalid module - no magic number present");
    }
    if (*magic != magicNumber) {
        throw Error("invalid module - wrong magic number");
    }
}


/* ------------------------- dump ------------------------- */

void Module::dump( ostream &os )
{
    os << modName << " (refs=" << refs << ",calls=" << calls << ")\n";
}


/* ------------------------- operator<< ------------------------- */

ostream& auction::operator<< ( ostream &os, Module &obj )
{
    obj.dump(os);
    return os;
}
