#include "remotehost.hh"

CLICK_DECLS

RemoteHost::RemoteHost(String _remoteHostID) {
    remoteHostID = _remoteHostID;
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(RemoteHost)