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
