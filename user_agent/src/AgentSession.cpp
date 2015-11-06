/// ----------------------------------------*- mode: C++; -*--
/// @file Session.cpp
/// The Session class.
/// ----------------------------------------------------------
/// $Id: Session.cpp 2558 2015-10-02 6:16:00 $
/// $HeadURL: https://./src/Session.cpp $
// ===========================================================
//                      
// Copyright (C) 2012-2014, all rights reserved by
// - System and Computing Engineering, Universidad de los Andes
//
// More information and contact:
// https://www.uniandes.edu.co/
//                      
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ===========================================================

#include "ParserFcts.h"
#include "AgentSession.h"
#include "Timeval.h"

using namespace auction;


/**
 * Constructor.
 *
 */
AgentSession::AgentSession(string _sessionId):
	Session(_sessionId)
{

    log = Logger::getInstance();
    ch = log->createChannel("AgentSession");	
}

/**
 * Destructor.
 *
 */
AgentSession::~AgentSession()
{
	
}

void AgentSession::setRequestData(string set, string name)
{
	resourceRequestSet = set;
	resourceRequestName = name;
}

std::ostream& operator<<(std::ostream &out, const AgentSession &obj) 
{
	using namespace std;

	out << obj.getSessionId();

	return out;
}


string AgentSession::getInfo()
{

	string str = Session::getInfo();
	str =  str + "Start:" + Timeval::toString(start);
	str =  str + " Stop:" + Timeval::toString(stop);
	return str;
}

void AgentSession::setAuctions(auctionSet_t &setParam)
{

#ifdef DEBUG    
    log->dlog(ch, "start Set Auctions");
#endif 

	auctionSetIter_t iter;
    for (iter = setParam.begin(); iter != setParam.end(); iter++) 
    {
		auctionSet.insert(*iter);  
	}

#ifdef DEBUG    
    log->dlog(ch, "end Set Auctions");
#endif 
	
}

auctionSet_t & AgentSession::getAuctions(void)
{
	return auctionSet;
}
