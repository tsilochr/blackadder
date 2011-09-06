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

#ifndef CLICK_SCOPE_HH
#define CLICK_SCOPE_HH

#include "informationitem.hh"
#include "helper.hh"

CLICK_DECLS

class Scope;
class RemoteHost;
class InformationItem;

class Scope {
public:
    Scope(unsigned char _strategy, Scope *father_scope);
    ~Scope();

    String printID();
    String printID(unsigned int i);
    
    bool updatePublishers(String ID, RemoteHost *_publisher);
    bool updateSubscribers(String ID, RemoteHost *_subscriber);
    
    void recursivelyUpdateIDs(ScopeHashMap &scopeIndex, IIHashMap &pubIndex, String suffixID);
    bool checkForOtherPubSub(Scope *fatherScope);
    void findSubscribers(RemoteHostSet &subscribers);
    void findAllInformationItems(InformationItemSet &_informationitems);
    void getSubscopes(ScopeSet & _subscopes);
    
    /*members*/
    IdsHashMap ids;
    bool isRoot;
    unsigned char strategy;
    ScopeSet fatherScopes;
    ScopeSet childrenScopes;
    InformationItemSet informationitems;
};

CLICK_ENDDECLS
#endif

