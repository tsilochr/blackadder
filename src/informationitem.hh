#ifndef CLICK_INFORMATIONITEM_HH
#define CLICK_INFORMATIONITEM_HH

#include "common.hh"

CLICK_DECLS

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

CLICK_ENDDECLS
#endif

