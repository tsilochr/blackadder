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

#ifndef CLICK_HELPER_HH
#define CLICK_HELPER_HH

#define PURSUIT_ID_LEN 8 //in bytes
#define FID_LEN 8 //in bytes
#define MAC_LEN 6
#define NODEID_LEN PURSUIT_ID_LEN //in bytes

/****some strategies*****/
#define NODE_LOCAL          0
#define LINK_LOCAL          1
#define DOMAIN_LOCAL        2
/*publish immediately using the provided FID (by the application or the click module)*/
#define PUBLISH_NOW         3
/*subscribe locally only..i.e. add an active scope or info item subscription..do not notify the RV*/
#define SUBSCRIBE_LOCALLY   4
/************************/

#define LOCAL_PROCESS 0
#define CLICK_MODULE  1

/*intra and inter click message types*/
#define PUBLISH_SCOPE 0
#define PUBLISH_INFO 1
#define UNPUBLISH_SCOPE 2
#define UNPUBLISH_INFO 3
#define SUBSCRIBE_SCOPE 4
#define SUBSCRIBE_INFO 5
#define UNSUBSCRIBE_SCOPE 6
#define UNSUBSCRIBE_INFO 7
#define PUBLISH_DATA  8 //the request
#define CONNECT 12
#define DISCONNECT 13


#define START_PUBLISH 100
#define STOP_PUBLISH 101
#define SCOPE_PUBLISHED 102
#define SCOPE_UNPUBLISHED 103
#define PUBLISHED_DATA 104
#define MATCH_PUB_SUBS 105

#define RV_ELEMENT 1 //put the correct click port here

#endif
