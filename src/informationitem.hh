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

#ifndef CLICK_INFORMATIONITEM_HH
#define CLICK_INFORMATIONITEM_HH

#include "common.hh"
#include "scope.hh"

#include <click/straccum.hh>

class RemoteHost;
class Scope;
class InformationItem;

class InformationItem {
public:
    InformationItem(unsigned char _strategy, Scope *father_scope);
    ~InformationItem();
    String printID();
    String printID(unsigned int i);
    void updateIDs(IIHashMap &pubIndex, String suffixID);

    bool updatePublishers(String ID, RemoteHost *_publisher);
    bool updateSubscribers(String ID, RemoteHost *_subscriber);
    
    bool checkForOtherPubSub(Scope *fatherScope);

    void findSubscribers(RemoteHostSet &subscribers);
    void findPublishers(RemoteHostSet &publishers);

    /*members*/
    IdsHashMap ids;
    RemoteHostSet fullSubscribersList;
    ScopeSet fatherScopes;
    unsigned char strategy;
};

#endif

