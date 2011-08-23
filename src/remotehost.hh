/*
* Copyright (C) 2010-2011  George Parisis and Dirk Trossen
* All rights reserved.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License version
* 2 as published by the Free Software Foundation.
*
* Alternatively, this software may be distributed under the terms of
* the BSD license.
*
* See LICENSE and COPYING for more details.
*/

#ifndef CLICK_REMOTEHOST_HH
#define CLICK_REMOTEHOST_HH

#include "helper.hh"
#include "scope.hh"
#include "informationitem.hh"
//#include <set>
#include "common.hh"

CLICK_DECLS

class Scope;
class InformationItem;

class RemoteHost {
public:
    /*methods*/
    RemoteHost(String _remoteHostID);
    /*members*/
    String remoteHostID;
    /*these are recorded by the RV point*/
    StringSet publishedScopes;
    StringSet publishedInformationItems;
    StringSet subscribedScopes;
    StringSet subscribedInformationItems;
};

CLICK_ENDDECLS

#endif
