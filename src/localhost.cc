#include "localhost.hh"
#include "helper.hh"

CLICK_DECLS

LocalHost::LocalHost(int _type, int _id) {
    type = _type;
    id = _id;
    switch (type) {
        case LOCAL_PROCESS:
            localHostID = "app" + String::make_numeric((uint64_t) id);
            break;
        case CLICK_MODULE:
            localHostID = "click" + String::make_numeric((uint64_t) id);
            break;
    }
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(LocalHost)
