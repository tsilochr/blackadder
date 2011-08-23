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

#ifndef CLICK_ACTIVEPUB_HH
#define CLICK_ACTIVEPUB_HH

#include "common.hh"

CLICK_DECLS

class LocalHost;

class ActivePublication {
public:
    ActivePublication(String _fullID, unsigned char _strategy, bool _isScope);
    ~ActivePublication();

    String fullID;
    unsigned char strategy;
    PublisherHashMap publishers;
    Vector<String> allKnownIDs;
    BABitvector FID_to_subscribers;
    BABitvector RVFID;
    bool isScope;
};

class ActiveSubscription {
public:
    ActiveSubscription(String _fullID, unsigned char _strategy, bool _isScope);
    ~ActiveSubscription();

    String fullID;
    unsigned char strategy;
    LocalHostSet subscribers;
    Vector<String> allKnownIDs;
    BABitvector RVFID;
    bool isScope;
};

CLICK_ENDDECLS

#endif
