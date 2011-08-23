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

#ifndef CLICK_LOCALHOST_HH
#define CLICK_LOCALHOST_HH

#include "common.hh"

CLICK_DECLS
 

class LocalHost {
public:
    /*methods*/
    LocalHost(int type, int id);
    /*members*/
    String localHostID;
    int type; // 0 - local process, 1 - click module
    int id;
    /*these are recorded by the proxy element in each node*/
    StringSet activePublications;
    StringSet activeSubscriptions;
};

CLICK_ENDDECLS

#endif
