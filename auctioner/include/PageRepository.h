
/*! \file  PageRepository.h

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
  
    This file contains the declaration for class PageRepository
    code based on netmate.
    
    $Id: PageRepository.h 748 2015-07-29 15:56:00Z amarentes $
*/

#ifndef _PAGE_REPOSITORY_H_
#define _PAGE_REPOSITORY_H_


#include "stdincpp.h"

namespace auction
{

//! url to page content map
typedef map<string,string>            repository_t;
typedef map<string,string>::iterator  repositoryIter_t;


/*! \short stores HTML pages (i.e. text strings) which can be found
           based on their URI name (path + page-name)
*/

class PageRepository
{
  private:

    //! maps url to page content
    repository_t  pages;

    //! maps url to filename
    repository_t  fileNames;

  public:

    PageRepository() {}

    ~PageRepository() {}

    //! FIXME document
    void addPage( string url, string content )
    { 
        pages[url] = content; 
    }

    void addPageFile( string url, string filename );

    string getPage( string url );

    string getFileName( string url );
};

}; // namespace auction  

#endif // _PAGE_REPOSITORY_H_
