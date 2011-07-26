#ifndef CLICK_HELPER_HH
#define CLICK_HELPER_HH

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/element.hh>

#include <click/string.hh>
#include <click/hashtable.hh>
#include <click/vector.hh>
#include <click/bitvector.hh>
#include <click/straccum.hh>

#include <clicknet/ip.h>
#include <clicknet/udp.h>
#include <click/etheraddress.hh>
#include <click/ipaddress.hh>

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
