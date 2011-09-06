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

#ifndef CLICK_COMMON_HH
#define CLICK_COMMON_HH

#include "ba_bitvector.hh"

#include <click/string.hh>
#include <click/hashtable.hh>

CLICK_DECLS

class LocalHost;
class RemoteHost;
class Scope;
class InformationItem;
class ActivePublication;
class ActiveSubscription;

struct LocalHostSetItem {
    /* data */
    LocalHost * _lhpointer;

    /*mandated for template*/
    typedef LocalHost * key_type;
    typedef LocalHost * key_const_reference;

    key_const_reference hashkey() const {
        return _lhpointer;
    }

    /*constructor*/
    LocalHostSetItem(LocalHost * lhp) : _lhpointer(lhp) {
    }
};

struct RemoteHostSetItem {
    /* data */
    RemoteHost * _rhpointer;
    /*mandated for template*/
    typedef RemoteHost * key_type;
    typedef RemoteHost * key_const_reference;

    key_const_reference hashkey() const {
        return _rhpointer;
    }

    /*constructor*/
    RemoteHostSetItem(RemoteHost * rhp) : _rhpointer(rhp) {
    }
};

struct StringSetItem {
    /* data */
    String _strData;

    /*mandated for template*/
    typedef String key_type;
    typedef String key_const_reference;

    key_const_reference hashkey() const {
        return _strData;
    }

    /*constructor*/
    StringSetItem(String str) : _strData(str) {

    }
};

struct InformationItemSetItem {
    /* data */
    InformationItem * _iipointer;

    /*mandated for template*/
    typedef InformationItem * key_type;
    typedef InformationItem * key_const_reference;

    key_const_reference hashkey() const {
        return _iipointer;
    }

    /*constructor*/
    InformationItemSetItem(InformationItem * iip) : _iipointer(iip) {
    }
};

struct ScopeSetItem {
    /* data */
    Scope * _scpointer;
    /*mandated for template*/
    typedef Scope * key_type;
    typedef Scope * key_const_reference;

    key_const_reference hashkey() const {
        return _scpointer;
    }

    /*constructor*/
    ScopeSetItem(Scope * scp) : _scpointer(scp) {
    }
};
typedef HashTable<LocalHostSetItem> LocalHostSet;
typedef LocalHostSet::iterator LocalHostSetIter;

typedef HashTable<RemoteHostSetItem> RemoteHostSet;
typedef RemoteHostSet::iterator RemoteHostSetIter;

typedef HashTable<InformationItemSetItem> InformationItemSet;
typedef InformationItemSet::iterator InformationItemSetIter;

typedef HashTable<ScopeSetItem> ScopeSet;
typedef ScopeSet::iterator ScopeSetIter;

typedef HashTable<StringSetItem> StringSet;
typedef StringSet::iterator StringSetIter;

typedef Pair<RemoteHostSet, RemoteHostSet> RemoteHostPair;

typedef HashTable<String, RemoteHostPair *> IdsHashMap;
typedef IdsHashMap::iterator IdsHashMapIter;

typedef HashTable<String, RemoteHost *> RemoteHostHashMap;
typedef RemoteHostHashMap::iterator RemoteHostHashMapIter;

typedef HashTable<String, InformationItem *> IIHashMap;
typedef IIHashMap::iterator IIHashMapIter;

typedef HashTable<String, Scope *> ScopeHashMap;
typedef ScopeHashMap::iterator ScopeHashMapIter;

typedef HashTable<RemoteHost *, BABitvector *> RemoteHostBitVectorHashMap;
typedef RemoteHostBitVectorHashMap::iterator RemoteHostBitVectorHashMapIter;

typedef HashTable<LocalHost *, String> LocalHostStringHashMap;
typedef LocalHostStringHashMap::iterator LocalHostStringHashMapIter;

typedef HashTable<LocalHost *, unsigned char> PublisherHashMap;
typedef PublisherHashMap::iterator PublisherHashMapIter;

typedef Pair<RemoteHost *, BABitvector *> RemoteHostBitVectorPair;

typedef HashTable<int, LocalHost *> PubSubIdx;
typedef PubSubIdx::iterator PubSubIdxIter;

typedef HashTable<String, ActivePublication *> ActivePub;
typedef ActivePub::iterator ActivePubIter;

typedef HashTable<String, ActiveSubscription *> ActiveSub;
typedef ActiveSub::iterator ActiveSubIter;

CLICK_ENDDECLS
#endif

