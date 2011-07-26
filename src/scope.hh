#ifndef CLICK_SCOPE_HH
#define CLICK_SCOPE_HH

#include "common.hh"


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

