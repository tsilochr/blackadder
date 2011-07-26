#include "activepub.hh"

CLICK_DECLS

ActivePublication::ActivePublication(String _fullID, unsigned char _strategy, bool _isScope) {
    fullID = _fullID;
    strategy = _strategy;
    isScope = _isScope;
}

ActivePublication::~ActivePublication() {
}

ActiveSubscription::ActiveSubscription(String _fullID, unsigned char _strategy, bool _isScope) {
    fullID = _fullID;
    strategy = _strategy;
    isScope = _isScope;
}

ActiveSubscription::~ActiveSubscription() {
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(ActivePublication)
ELEMENT_PROVIDES(ActiveSubscription)
