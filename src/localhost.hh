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
