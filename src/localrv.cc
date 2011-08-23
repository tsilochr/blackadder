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

#include "localrv.hh"

CLICK_DECLS

LocalRV::LocalRV() {

}

LocalRV::~LocalRV() {
}

int LocalRV::configure(Vector<String> &conf, ErrorHandler *errh) {
    gc = (GlobalConf *) cp_element(conf[0], this);
    cp_integer(conf[1], &number_of_neighbours);
    click_chatter("number_of_neighbours: %d", number_of_neighbours);
    for (int i = 0; i < number_of_neighbours; i++) {
        String node_label = conf[2 + 3 * i];
        String LID = conf[3 + 3 * i];
        String iLID = conf[4 + 3 * i];
        Neighbour * neigh = new Neighbour(node_label, LID, iLID);
        neighbours.set(node_label, neigh);
        click_chatter("Added Neighbour %s, LID to: %s, iLID: %s", neigh->label.c_str(), neigh->LID.to_string().c_str(), neigh->iLID.to_string().c_str());
    }
    //click_chatter("LocalRV: configured!");
    return 0;
}

int LocalRV::initialize(ErrorHandler *errh) {
    unsigned char type = SUBSCRIBE_SCOPE;
    unsigned char strategy = SUBSCRIBE_LOCALLY;
    unsigned char id_len = PURSUIT_ID_LEN / PURSUIT_ID_LEN;
    unsigned char prefix_id_len = 0;
    WritablePacket *p = Packet::make(100);
    localProxy = createRemoteHost(gc->nodeID);
    /*First time I am doing that.. I will send a subscription (SUBSCRIBE_LOCALLY) to the localproxy during my initialization*/
    memcpy(p->data(), &type, sizeof (type));
    memcpy(p->data() + sizeof (type), &id_len, sizeof (id_len));
    memcpy(p->data() + sizeof (type) + sizeof (id_len), gc->RVScope.c_str(), gc->RVScope.length());
    memcpy(p->data() + sizeof (type) + sizeof (id_len) + gc->RVScope.length(), &prefix_id_len, sizeof (prefix_id_len));
    memcpy(p->data() + sizeof (type) + sizeof (id_len) + gc->RVScope.length() + sizeof (prefix_id_len), &strategy, sizeof (strategy));
    output(0).push(p);
    //click_chatter("LocalRV: initialized!");
    return 0;
}

void LocalRV::cleanup(CleanupStage stage) {
    neighbours.clear();
    for (RemoteHostHashMapIter it = pub_sub_Index.begin(); it != pub_sub_Index.end(); it++) {
        delete (*it).second;
    }
    for (ScopeHashMapIter it = scopeIndex.begin(); it != scopeIndex.end(); it++) {
        delete (*it).second;
    }
    for (IIHashMapIter it = pubIndex.begin(); it != pubIndex.end(); it++) {
        delete (*it).second;
    }
    //click_chatter("LocalRV: Cleaned Up!");
}

void LocalRV::push(int in_port, Packet * p) {
    RemoteHost *_remotehost;
    /*For now this events are going to be PUBLISHED_DATA only!!*/
    unsigned char type, typeOfAPIEvent;
    unsigned char IDLengthOfAPIEvent;
    String IDOfAPIEvent;
    /***********************************************************/
    String ID, prefixID;
    String nodeID;
    unsigned char IDLength/*in fragments of PURSUIT_ID_LEN each*/, prefixIDLength/*in fragments of PURSUIT_ID_LEN each*/, strategy;
    if (in_port == 0) {
        /*Since the last changes in 23/06/2011 the locaRV Element "communicates" with the "world" using the universal blackadder API, exactly like an application*/
        typeOfAPIEvent = *(p->data());
        IDLengthOfAPIEvent = *(p->data() + sizeof (typeOfAPIEvent));
        IDOfAPIEvent = String((const char *) (p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent)), IDLengthOfAPIEvent * PURSUIT_ID_LEN);
        if (typeOfAPIEvent == PUBLISHED_DATA) {
            nodeID = IDOfAPIEvent.substring(PURSUIT_ID_LEN);
            type = *(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + IDLengthOfAPIEvent * PURSUIT_ID_LEN);
            IDLength = *(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + IDLengthOfAPIEvent * PURSUIT_ID_LEN + sizeof (type));
            ID = String((const char *) (p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + IDLengthOfAPIEvent * PURSUIT_ID_LEN + sizeof (type) + sizeof (IDLength)), IDLength * PURSUIT_ID_LEN);
            prefixIDLength = *(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + IDLengthOfAPIEvent * PURSUIT_ID_LEN + sizeof (type) + sizeof (IDLength) + ID.length());
            prefixID = String((const char *) (p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + IDLengthOfAPIEvent * PURSUIT_ID_LEN + sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (prefixIDLength)), prefixIDLength * PURSUIT_ID_LEN);
            strategy = *(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + IDLengthOfAPIEvent * PURSUIT_ID_LEN + sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (prefixIDLength) + prefixID.length());
            _remotehost = createRemoteHost(nodeID);
            switch (type) {
                case PUBLISH_SCOPE:
                    //click_chatter("LocalRV: received publish_scope request: %s, %s, %s, %d", _remotehost->remoteHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
                    publish_scope(_remotehost, ID, prefixID, strategy);
                    break;
                case PUBLISH_INFO:
                    //click_chatter("LocalRV: received publish_info request: %s, %s, %s, %d", _remotehost->remoteHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
                    publish_info(_remotehost, ID, prefixID, strategy);
                    break;
                case UNPUBLISH_SCOPE:
                    //click_chatter("LocalRV: received unpublish_scope request: %s, %s, %s, %d", _remotehost->remoteHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
                    unpublish_scope(_remotehost, ID, prefixID, strategy);
                    break;
                case UNPUBLISH_INFO:
                    //click_chatter("LocalRV: received unpublish_info request: %s, %s, %s, %d", _remotehost->remoteHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
                    unpublish_info(_remotehost, ID, prefixID, strategy);
                    break;
                case SUBSCRIBE_SCOPE:
                    //click_chatter("LocalRV: received subscribe_scope request: %s, %s, %s, %d", _remotehost->remoteHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
                    subscribe_scope(_remotehost, ID, prefixID, strategy);
                    break;
                case SUBSCRIBE_INFO:
                    //click_chatter("LocalRV: received subscribe_info request: %s, %s, %s, %d", _remotehost->remoteHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
                    subscribe_info(_remotehost, ID, prefixID, strategy);
                    break;
                case UNSUBSCRIBE_SCOPE:
                    //click_chatter("LocalRV: received unsubscribe_scope request: %s, %s, %s, %d", _remotehost->remoteHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
                    unsubscribe_scope(_remotehost, ID, prefixID, strategy);
                    break;
                case UNSUBSCRIBE_INFO:
                    //click_chatter("LocalRV: received unsubscribe_info request: %s, %s, %s, %d", _remotehost->remoteHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
                    unsubscribe_info(_remotehost, ID, prefixID, strategy);
                    break;
                default:
                    click_chatter("LocalRV: unknown request type - skipping request");
                    break;
            }
            p->kill();
        } else {
            click_chatter("LocalRV: FATAL - I am expecting only PUBLISHED_DATA pub/sub events");
        }
    } else {
        click_chatter("LocalRV: I am not expecting packets from other click ports - FATAL");
    }
}

void LocalRV::publish_scope(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy) {
    /*When a Scope is published the RV point (that is this node) should notify interested subscribers about the new scope*/
    /*For each subscriber the RV point should use the appropriate ID path*/
    if ((prefixID.length() == 0) && (ID.length() == PURSUIT_ID_LEN)) {
        publish_root_scope(_publisher, ID, strategy);
    } else if ((prefixID.length() > 0) && (ID.length() == PURSUIT_ID_LEN)) {
        publish_inner_scope(_publisher, ID, prefixID, strategy);
    } else if ((prefixID.length() > 0) && (ID.length() > PURSUIT_ID_LEN)) {
        republish_inner_scope(_publisher, ID, prefixID, strategy);
    } else {
        click_chatter("LocalRV: error while publishing scope. ID: %s - prefixID: %s", ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str());
    }
}

void LocalRV::publish_root_scope(RemoteHost *_publisher, String &ID, unsigned char &strategy) {
    /*when root scopes are published there is no need to notify subscribers*/
    Scope *sc = scopeIndex.get(ID);
    if (sc == scopeIndex.default_value()) {
        sc = new Scope(strategy, NULL);
        scopeIndex.set(ID, sc);
        if (sc->updatePublishers(ID, _publisher)) {
            /*add the scope to the publisher's set*/
            _publisher->publishedScopes.find_insert(StringSetItem(ID));
            click_chatter("LocalRV: added publisher %s to (new) scope: %s(%d)", _publisher->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
        }
    } else {
        /*check if the strategies match*/
        if (sc->strategy == strategy) {
            if (sc->updatePublishers(ID, _publisher)) {
                /*add the scope to the publisher's set*/
                _publisher->publishedScopes.find_insert(StringSetItem(ID));
                click_chatter("LocalRV: added publisher %s to scope: %s(%d)", _publisher->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
            }
        } else {
            click_chatter("LocalRV: strategies don't match....aborting");
        }
    }
}

void LocalRV::publish_inner_scope(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy) {
    Scope *sc;
    Scope *fatherScope;
    String fullID;
    /*the publisher publishes a scope (a single fragment ID is used) under a path that must exist*/
    /*check if a InformationItem with the same path_id exists*/
    fullID = prefixID + ID;
    if (pubIndex.find(fullID) == pubIndex.end()) {
        /*check if the father scope exists*/
        fatherScope = scopeIndex.get(prefixID);
        if (fatherScope != scopeIndex.default_value()) {
            /*check if the scope under publication exists..*/
            sc = scopeIndex.get(fullID);
            if (sc == scopeIndex.default_value()) {
                /*it does not exist...create a new scope*/
                /*check the strategy of the father scope*/
                if (fatherScope->strategy == strategy) {
                    sc = new Scope(strategy, fatherScope);
                    sc->recursivelyUpdateIDs(scopeIndex, pubIndex, fullID.substring(fullID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN));
                    if (sc->updatePublishers(fullID, _publisher)) {
                        /*add the scope to the publisher's set*/
                        _publisher->publishedScopes.find_insert(StringSetItem(fullID));
                        click_chatter("LocalRV: added publisher %s to (new) scope: %s(%d)", _publisher->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                        /*notify subscribers here!!*/
                        /*differently compared to the republish_inner_scope case*/
                        RemoteHostSet subscribers;
                        fatherScope->findSubscribers(subscribers);
                        notifySubscribers(sc, subscribers);
                    }
                } else {
                    click_chatter("LocalRV: error while publishing scope - father scope %s has incompatible strategy...", fatherScope->printID().c_str());
                }
            } else {
                if (sc->strategy == strategy) {
                    if (sc->updatePublishers(fullID, _publisher)) {
                        /*add the scope to the publisher's set*/
                        _publisher->publishedScopes.find_insert(StringSetItem(fullID));
                        /*DO NOT notify subscribers - they already know about that scope!!*/
                        click_chatter("LocalRV: added publisher %s to scope: %s(%d)", _publisher->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                    }
                } else {
                    click_chatter("LocalRV: scope %s exists..but with a different strategy", sc->printID().c_str());
                }
            }
        } else {
            click_chatter("LocalRV: error while publishing scope %s under %s which does not exist!", ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str());
        }
    } else {
        click_chatter("LocalRV: error - a piece of info with the same path_id exists");
    }
}

void LocalRV::republish_inner_scope(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy) {
    Scope *equivalentScope;
    Scope *existingScope;
    Scope *fatherScope;
    /*The publisher republishes an inner scope under an existing scope*/
    String suffixID = ID.substring(ID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
    String fullID = prefixID + suffixID;
    String existingPrefixID = ID.substring(0, ID.length() - PURSUIT_ID_LEN);
    if (pubIndex.find(fullID) == pubIndex.end()) {
        /*the publisher publishes an existing scope under a path that must exist*/
        fatherScope = scopeIndex.get(prefixID);
        if (fatherScope != scopeIndex.default_value()) {
            existingScope = scopeIndex.get(ID);
            if (existingScope != scopeIndex.default_value()) {
                equivalentScope = scopeIndex.get(fullID);
                if (equivalentScope == scopeIndex.default_value()) {
                    if (fatherScope->strategy == strategy) {
                        existingScope->fatherScopes.find_insert(ScopeSetItem(fatherScope));
                        fatherScope->childrenScopes.find_insert(ScopeSetItem(existingScope));
                        existingScope->recursivelyUpdateIDs(scopeIndex, pubIndex, suffixID);
                        if (existingScope->updatePublishers(fullID, _publisher)) {
                            /*add the scope to the publisher's set*/
                            _publisher->publishedScopes.find_insert(StringSetItem(fullID));
                            click_chatter("LocalRV: added publisher %s to republished scope%s under scope %s(%d)", _publisher->remoteHostID.c_str(), existingScope->printID().c_str(), fatherScope->printID().c_str(), (int) strategy);
                            /*notify subscribers here - careful to use the right father as a start!!*/
                            RemoteHostSet subscribers;
                            fatherScope->findSubscribers(subscribers);
                            /*existingPrefixID must be excluded*/
                            //click_chatter("existingPrefixID: %s", existingPrefixID.quoted_hex().c_str());
                            notifySubscribers(existingScope, existingPrefixID, subscribers);
                        }
                    } else {
                        click_chatter("LocalRV: error while republishing father scope %s has incompatible strategy...", fatherScope->printID().c_str());
                    }
                } else {
                    if (equivalentScope->strategy == strategy) {
                        if (equivalentScope->updatePublishers(fullID, _publisher)) {
                            /*add the scope to the publisher's set*/
                            _publisher->publishedScopes.find_insert(StringSetItem(fullID));
                            /*DO NOT notify subscribers - they already know about that scope (the republication)!!*/
                            click_chatter("LocalRV: added publisher %s to scope: %s(%d)", _publisher->remoteHostID.c_str(), equivalentScope->printID().c_str(), (int) strategy);
                        }
                    } else {
                        click_chatter("LocalRV: scope %s exists..but with a different strategy", equivalentScope->printID().c_str());
                    }
                }
            } else {
                click_chatter("LocalRV: error - cannot (re)publish scope %s somewhere else because it doesn't exist", ID.quoted_hex().c_str());
            }
        } else {
            click_chatter("LocalRV: Error - cannot (re)publish scope %s under %s which does not exist!", ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str());
        }
    } else {
        click_chatter("LocalRV: Error - A piece of info with the same ID exists");
    }
}

void LocalRV::publish_info(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy) {
    if ((prefixID.length() > 0) && (ID.length() == PURSUIT_ID_LEN)) {
        advertise_info(_publisher, ID, prefixID, strategy);
    } else if ((prefixID.length() > 0) && (ID.length() > PURSUIT_ID_LEN)) {
        readvertise_info(_publisher, ID, prefixID, strategy);
    } else {
        click_chatter("LocalRV: error while publishing information. ID: %s - prefixID: %s", ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str());
    }
}

void LocalRV::advertise_info(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy) {
    InformationItem *pub;
    Scope *fatherScope;
    String fullID = prefixID + ID;
    /*the publisher advertises a piece (a single ID fragment) of info under a path that must exist*/
    fatherScope = scopeIndex.get(prefixID);
    if (fatherScope != scopeIndex.default_value()) {
        /*check if the InformationItem (with this specific ID) is already there...*/
        pub = pubIndex.get(fullID);
        if (pub == pubIndex.default_value()) {
            /*check if a scope with the same ID exists*/
            if (scopeIndex.find(fullID) == scopeIndex.end()) {
                if (fatherScope->strategy == strategy) {
                    pub = new InformationItem(fatherScope->strategy, fatherScope);
                    pub->updateIDs(pubIndex, fullID.substring(fullID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN));
                    if (pub->updatePublishers(fullID, _publisher)) {
                        /*add the InformationItem to the publisher's set*/
                        _publisher->publishedInformationItems.find_insert(StringSetItem(fullID));
                        click_chatter("LocalRV: added publisher %s to (new) InformationItem: %s(%d)", _publisher->remoteHostID.c_str(), pub->printID().c_str(), (int) strategy);
                        RemoteHostSet subscribers;
                        pub->findSubscribers(subscribers);
                        fatherScope->findSubscribers(subscribers);
                        rendezvous(pub, subscribers);
                    }
                } else {
                    click_chatter("LocalRV: Error could not add InformationItem - strategy mismatch");
                }
            } else {
                click_chatter("LocalRV: Error - a scope with the same ID exists");
            }
        } else {
            if (fatherScope->strategy == strategy) {
                if (pub->updatePublishers(fullID, _publisher)) {
                    /*add the InformationItem to the publisher's set*/
                    _publisher->publishedInformationItems.find_insert(StringSetItem(fullID));
                    click_chatter("LocalRV: added publisher %s to InformationItem: %s(%d)", _publisher->remoteHostID.c_str(), pub->printID().c_str(), (int) strategy);
                    RemoteHostSet subscribers;
                    pub->findSubscribers(subscribers);
                    /*careful here...this pub MAY have multiple fathers*/
                    for (ScopeSetIter fathersc_it = pub->fatherScopes.begin(); fathersc_it != pub->fatherScopes.end(); fathersc_it++) {
                        (*fathersc_it)._scpointer->findSubscribers(subscribers);
                    }
                    rendezvous(pub, subscribers);
                }
            } else {
                click_chatter("LocalRV: Error could not update InformationItem - strategy mismatch");
            }
        }
    } else {
        click_chatter("LocalRV: Error - Scope prefix %s doesn't exist", prefixID.quoted_hex().c_str());
    }
}

void LocalRV::readvertise_info(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy) {
    InformationItem *existingPub;
    InformationItem *equivalentPub;
    Scope *fatherScope;
    String suffixID = ID.substring(ID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
    String fullID = prefixID + suffixID;
    /*the publisher re-advertises an existing InformationItem under a path that must exist*/
    /*in this case the InformationItem will have more than one IDs*/
    /*check if a scope with the same ID exists*/
    if (scopeIndex.find(fullID) == scopeIndex.end()) {
        fatherScope = scopeIndex.get(prefixID);
        /*check if the parent scope exists*/
        if (fatherScope != scopeIndex.default_value()) {
            /*check if the InformationItem exists..*/
            existingPub = pubIndex.get(ID);
            if (existingPub != pubIndex.default_value()) {
                /*check if the InformationItem under the new path exists*/
                equivalentPub = pubIndex.get(fullID);
                if (equivalentPub == pubIndex.default_value()) {
                    if (fatherScope->strategy == strategy) {
                        existingPub->fatherScopes.find_insert(ScopeSetItem(fatherScope));
                        fatherScope->informationitems.find_insert(InformationItemSetItem(existingPub));
                        existingPub->updateIDs(pubIndex, suffixID);
                        if (existingPub->updatePublishers(fullID, _publisher)) {
                            /*add the InformationItem to the publisher's set*/
                            _publisher->publishedInformationItems.find_insert(StringSetItem(fullID));
                            click_chatter("LocalRV: added publisher %s to readvertised InformationItem %s under path %s (%d)", _publisher->remoteHostID.c_str(), existingPub->printID().c_str(), fatherScope->printID().c_str(), (int) strategy);
                            RemoteHostSet subscribers;
                            existingPub->findSubscribers(subscribers);
                            /*careful here...I have multiple fathers*/
                            for (ScopeSetIter fathersc_it = existingPub->fatherScopes.begin(); fathersc_it != existingPub->fatherScopes.end(); fathersc_it++) {
                                (*fathersc_it)._scpointer->findSubscribers(subscribers);
                            }
                            rendezvous(existingPub, subscribers);
                        }
                    } else {
                        click_chatter("LocalRV: Error could not add InformationItem- strategy mismatch");
                    }
                } else {
                    if (fatherScope->strategy == strategy) {
                        if (equivalentPub->updatePublishers(fullID, _publisher)) {
                            /*add the InformationItem to the publisher's set*/
                            _publisher->publishedInformationItems.find_insert(StringSetItem(fullID));
                            click_chatter("LocalRV: added publisher %s to InformationItem: %s(%d)", _publisher->remoteHostID.c_str(), equivalentPub->printID().c_str(), (int) strategy);
                            RemoteHostSet subscribers;
                            equivalentPub->findSubscribers(subscribers);
                            /*careful here...I have multiple fathers*/
                            for (ScopeSetIter fathersc_it = equivalentPub->fatherScopes.begin(); fathersc_it != equivalentPub->fatherScopes.end(); fathersc_it++) {
                                (*fathersc_it)._scpointer->findSubscribers(subscribers);
                            }
                            rendezvous(equivalentPub, subscribers);
                        }
                    } else {
                        click_chatter("LocalRV: Error could not republish InformationItem - strategy mismatch");
                    }
                }
            } else {
                click_chatter("LocalRV: Error - cannot (re)advertise info %s somewhere else because it doesn't exist", ID.quoted_hex().c_str());
            }
        } else {
            click_chatter("LocalRV: Error - (re)advertise info under %s that doesn't exist!", prefixID.quoted_hex().c_str());
        }
    } else {
        click_chatter("LocalRV: Error - a scope with the same ID exists");
    }
}

void LocalRV::unpublish_scope(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy) {
    Scope *sc;
    Scope *fatherScope;
    String fullID = prefixID + ID;
    sc = scopeIndex.get(fullID);
    if (sc != scopeIndex.default_value()) {
        if (sc->strategy == strategy) {
            fatherScope = scopeIndex.get(prefixID);
            if (fatherScope != scopeIndex.default_value()) {
                /*not a root scope*/
                /*try to unpublish all InformationItems under that scope*/
                for (InformationItemSetIter pub_it = sc->informationitems.begin(); pub_it != sc->informationitems.end(); pub_it++) {
                    InformationItem *pub = (*pub_it)._iipointer;
                    String pubSuffixID = (*pub->ids.begin()).first.substring((*pub->ids.begin()).first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                    /*call unpublish_info() for all IDs of this scope*/
                    for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
                        String scPrefixID = (*it).first;
                        unpublish_info(_publisher, pubSuffixID, scPrefixID, strategy);
                    }
                }
                RemoteHostPair * pair = sc->ids.get(fullID);
                /*erase _publisher (if it exists) from the appropriate ID pair*/
                if (pair->first.find(_publisher) != pair->first.end()) {
                    pair->first.erase(_publisher);
                    _publisher->publishedScopes.erase(fullID);
                    /*do not try to delete if there are subscopes or InformationItems under the scope*/
                    if ((sc->childrenScopes.size() == 0) && (sc->informationitems.size() == 0)) {
                        /*different approach is followed depending on the number of father scopes (NOT on the number of IDS)*/
                        if (sc->fatherScopes.size() == 1) {
                            //click_chatter("LocalRV: scope has a single father scope");
                            if (!sc->checkForOtherPubSub(fatherScope)) {
                                /*safe to delete Scope*/
                                fatherScope->childrenScopes.erase(sc);
                                click_chatter("LocalRV: deleted publisher %s from (deleted) scope %s (%d)", _publisher->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                                /*delete all IDs from scopeIndex*/
                                for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
                                    scopeIndex.erase(it.key());
                                }
                                delete sc;
                            } else {
                                //click_chatter("LocalRV: deleted publisher %s from scope %s(%d)", _publisher->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                            }
                        } else {
                            //click_chatter("scope has many father scopes");
                            if (!sc->checkForOtherPubSub(fatherScope)) {
                                /*safe to delete scope (only this the specific branch)*/
                                fatherScope->childrenScopes.erase(sc);
                                sc->fatherScopes.erase(fatherScope);
                                click_chatter("LocalRV: deleted publisher %s from (deleted) scope branch %s(%d)", _publisher->remoteHostID.c_str(), fullID.quoted_hex().c_str(), (int) strategy);
                                /*delete all IDs from scopeIndex*/
                                String suffixID = (*sc->ids.begin()).first.substring((*sc->ids.begin()).first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                                for (IdsHashMapIter it = fatherScope->ids.begin(); it != fatherScope->ids.end(); it++) {
                                    /*since the scope is not deleted, manually delete this pair*/
                                    delete sc->ids.get((*it).first + suffixID);
                                    sc->ids.erase((*it).first + suffixID);
                                    scopeIndex.erase((*it).first + suffixID);
                                }
                            } else {
                                //click_chatter("LocalRV: deleted publisher %s from scope branch %s(%d)", _publisher->remoteHostID.c_str(), fullID.quoted_hex().c_str(), (int) strategy);
                            }
                        }
                    } else {
                        click_chatter("LocalRV: deleted publisher %s from scope %s (%d)", _publisher->remoteHostID.c_str(), fullID.quoted_hex().c_str(), (int) strategy);
                    }
                }
            } else {
                /*a ROOT scope*/
                /*try to unpublish all InformationItems under that scope*/
                InformationItemSetIter pub_it;
                for (pub_it = sc->informationitems.begin(); pub_it != sc->informationitems.end(); pub_it++) {
                    InformationItem *pub = (*pub_it)._iipointer;
                    String pubSuffixID = (*pub->ids.begin()).first.substring((*pub->ids.begin()).first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                    /*call unpublish_info() for all IDs of this scope*/
                    for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
                        String scPrefixID = (*it).first;
                        unpublish_info(_publisher, pubSuffixID, scPrefixID, strategy);
                    }
                }
                RemoteHostPair * pair = sc->ids.get(fullID);
                /*erase _publisher (if it exists) from the appropriate ID pair*/
                if (pair->first.find(_publisher) != pair->first.end()) {
                    pair->first.erase(_publisher);
                    _publisher->publishedScopes.erase(fullID);
                    /*do not try to delete if there are subscopes or InformationItems under the scope*/
                    if ((sc->childrenScopes.size() == 0) && (sc->informationitems.size() == 0)) {
                        if (!sc->checkForOtherPubSub(NULL)) {
                            click_chatter("LocalRV: deleted publisher %s from (deleted) scope %s (%d)", _publisher->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                            /*delete all IDs from scopeIndex*/
                            for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
                                scopeIndex.erase((*it).first);
                            }
                            delete sc;
                        } else {
                            click_chatter("LocalRV: deleted publisher %s from scope %s (%d)", _publisher->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                        }
                    } else {
                        click_chatter("LocalRV: deleted publisher %s from scope %s (%d)", _publisher->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                    }
                }
            }
        } else {
            click_chatter("LocalRV: Cannot unpublish scope %s..strategy mismatch", fullID.quoted_hex().c_str());
        }
    } else {
        click_chatter("LocalRV: Scope %s does not exist...unpublish what?", fullID.quoted_hex().c_str());
    }
}

void LocalRV::unpublish_info(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy) {
    InformationItem *pub;
    Scope *fatherScope;
    String fullID = prefixID + ID;
    /*check if the publisher exists in general and get a pointer to the object*/
    pub = pubIndex.get(fullID);
    fatherScope = scopeIndex.get(prefixID);
    if (pub != pubIndex.default_value()) {
        if (fatherScope->strategy == strategy) {
            RemoteHostPair * pair = pub->ids.get(fullID);
            /*erase _publisher (if it exists) from the appropriate ID pair*/
            if (pair->first.find(_publisher) != pair->first.end()) {
                pair->first.erase(_publisher);
                _publisher->publishedInformationItems.erase(fullID);
                /*different approach is followed depending on the number of father scopes (NOT on the number of IDS)*/
                if (pub->fatherScopes.size() == 1) {
                    //click_chatter("LocalRV: info has a single father scope");
                    if (!pub->checkForOtherPubSub(fatherScope)) {
                        /*safe to delete InformationItem*/
                        fatherScope->informationitems.erase(pub);
                        click_chatter("LocalRV: deleted publisher %s from (deleted) InformationItem %s (%d)", _publisher->remoteHostID.c_str(), pub->printID().c_str(), (int) strategy);
                        /*delete all IDs from pubIndex*/
                        for (IdsHashMapIter it = pub->ids.begin(); it != pub->ids.end(); it++) {
                            pubIndex.erase((*it).first);
                        }
                        delete pub;
                    } else {
                        click_chatter("LocalRV: deleted publisher %s from InformationItem %s(%d)", _publisher->remoteHostID.c_str(), pub->printID().c_str(), (int) strategy);
                        /*do the rendezvous again*/
                        RemoteHostSet subscribers;
                        pub->findSubscribers(subscribers);
                        /*careful here...I have multiple fathers*/
                        for (ScopeSetIter fathersc_it = pub->fatherScopes.begin(); fathersc_it != pub->fatherScopes.end(); fathersc_it++) {
                            (*fathersc_it)._scpointer->findSubscribers(subscribers);
                        }
                        rendezvous(pub, subscribers);
                    }
                } else {
                    //click_chatter("info has many father scopes");
                    if (!pub->checkForOtherPubSub(fatherScope)) {
                        /*safe to delete InformationItem*/
                        fatherScope->informationitems.erase(pub);
                        pub->fatherScopes.erase(fatherScope);
                        click_chatter("LocalRV: deleted publisher %s from (deleted) InformationItem branch %s(%d)", _publisher->remoteHostID.c_str(), fullID.quoted_hex().c_str(), (int) strategy);
                        /*delete all IDs from pubIndex*/
                        String suffixID = (*pub->ids.begin()).first.substring((*pub->ids.begin()).first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                        for (IdsHashMapIter it = fatherScope->ids.begin(); it != fatherScope->ids.end(); it++) {
                            /*since the pub is not deleted, manually delete this pair*/
                            delete pub->ids.get((*it).first + suffixID);
                            pub->ids.erase((*it).first + suffixID);
                            pubIndex.erase((*it).first + suffixID);
                        }
                    } else {
                        //click_chatter("LocalRV: deleted publisher %s from InformationItem branch %s(%d)", _publisher->remoteHostID.c_str(), fullID.quoted_hex().c_str(), (int) strategy);
                    }
                    /*do the rendezvous again*/
                    RemoteHostSet subscribers;
                    pub->findSubscribers(subscribers);
                    /*careful here...I have multiple fathers*/
                    for (ScopeSetIter fathersc_it = pub->fatherScopes.begin(); fathersc_it != pub->fatherScopes.end(); fathersc_it++) {
                        (*fathersc_it)._scpointer->findSubscribers(subscribers);
                    }
                    rendezvous(pub, subscribers);
                    /********************************************/
                }
            }
        } else {
            click_chatter("LocalRV:Cannot unpublish %s..strategy mismatch", pub->printID().c_str());
        }
    } else {
        click_chatter("LocalRV:InformationItem %s does not exist...unpublish what", fullID.quoted_hex().c_str());
    }
}

void LocalRV::subscribe_scope(RemoteHost *_subscriber, String &ID, String &prefixID, unsigned char &strategy) {
    if ((prefixID.length() == 0) && (ID.length() == PURSUIT_ID_LEN)) {
        subscribe_root_scope(_subscriber, ID, strategy);
    } else if ((prefixID.length() > 0) && (ID.length() == PURSUIT_ID_LEN)) {
        subscribe_inner_scope(_subscriber, ID, prefixID, strategy);
    } else {
        click_chatter("LocalRV: error while subscribing to scope. ID: %s - prefixID: %s", ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str());
    }
}

void LocalRV::subscribe_root_scope(RemoteHost *_subscriber, String &ID, unsigned char &strategy) {
    Scope *sc = NULL;
    sc = scopeIndex.get(ID);
    if (sc == scopeIndex.default_value()) {
        /*the root scope does not exist. Create it and add subscription*/
        sc = new Scope(strategy, NULL);
        scopeIndex.set(ID, sc);
        if (sc->updateSubscribers(ID, _subscriber)) {
            /*add the scope to the subscriber's set*/
            _subscriber->subscribedScopes.find_insert(StringSetItem(ID));
            click_chatter("LocalRV: added subscriber %s to (new) scope %s (%d)", _subscriber->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
        }
    } else {
        /*check if the strategies match*/
        if (sc->strategy == strategy) {
            if (sc->updateSubscribers(ID, _subscriber)) {
                /*add the scope to the subscriber's set*/
                _subscriber->subscribedScopes.find_insert(StringSetItem(ID));
                click_chatter("LocalRV: added subscriber %s to scope %s(%d)", _subscriber->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                /*first notify the subscriber about the existing subscopes*/
                RemoteHostSet subscribers;
                ScopeSet _subscopes;
                subscribers.find_insert(RemoteHostSetItem(_subscriber));
                sc->getSubscopes(_subscopes);
                for (ScopeSetIter sc_set_it = _subscopes.begin(); sc_set_it != _subscopes.end(); sc_set_it++) {
                    notifySubscribers((*sc_set_it)._scpointer, subscribers);
                }
                /*then find all InformationItems for which the _subscriber is interested in*/
                InformationItemSet _informationitems;
                sc->findAllInformationItems(_informationitems);
                /*then, for each one do the rendez-vous process*/
                InformationItemSetIter pub_it;
                for (pub_it = _informationitems.begin(); pub_it != _informationitems.end(); pub_it++) {
                    RemoteHostSet subscribers;
                    (*pub_it)._iipointer->findSubscribers(subscribers);
                    /*careful here...I have multiple fathers*/
                    for (ScopeSetIter fathersc_it = (*pub_it)._iipointer->fatherScopes.begin(); fathersc_it != (*pub_it)._iipointer->fatherScopes.end(); fathersc_it++) {
                        (*fathersc_it)._scpointer->findSubscribers(subscribers);
                    }
                    rendezvous((*pub_it)._iipointer, subscribers);
                }
            }
        } else {
            click_chatter("LocalRV: strategies don't match....aborting subscription");
        }
    }
}

void LocalRV::subscribe_inner_scope(RemoteHost *_subscriber, String &ID, String &prefixID, unsigned char &strategy) {
    Scope *sc;
    Scope *fatherScope;
    String fullID;
    /*the publisher publishes a scope (a single fragment ID is used) under a path that must exist*/
    /*check if a InformationItem with the same path_id exists*/
    fullID = prefixID + ID;
    if (pubIndex.find(fullID) == pubIndex.end()) {
        /*check if the father scope exists*/
        fatherScope = scopeIndex.get(prefixID);
        if (fatherScope != scopeIndex.default_value()) {
            /*check if the scope under publication exists..*/
            sc = scopeIndex.get(fullID);
            if (sc == scopeIndex.default_value()) {
                /*it does not exist...create a new scope and add subscription*/
                /*check the strategy of the father scope*/
                if (fatherScope->strategy == strategy) {
                    sc = new Scope(strategy, fatherScope);
                    sc->recursivelyUpdateIDs(scopeIndex, pubIndex, fullID.substring(fullID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN));
                    if (sc->updateSubscribers(fullID, _subscriber)) {
                        /*add the scope to the publisher's set*/
                        _subscriber->subscribedScopes.find_insert(StringSetItem(fullID));
                        click_chatter("LocalRV: added subscriber %s to (new) scope %s(%d)", _subscriber->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                        /*WEIRD BUT notify other subscribers since the scope has been created!!*/
                        RemoteHostSet subscribers;
                        fatherScope->findSubscribers(subscribers);
                        notifySubscribers(sc, subscribers);
                    }
                } else {
                    click_chatter("LocalRV: error while subscribing to scope - father scope %s has incompatible strategy...", fatherScope->printID().c_str());
                }
            } else {
                if (sc->strategy == strategy) {
                    if (sc->updateSubscribers(fullID, _subscriber)) {
                        /*add the scope to the subscriber's set*/
                        _subscriber->subscribedScopes.find_insert(StringSetItem(fullID));
                        click_chatter("LocalRV: added subscriber %s to scope %s(%d)", _subscriber->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                        /*first notify the subscriber about the existing subscopes*/
                        RemoteHostSet subscribers;
                        ScopeSet _subscopes;
                        subscribers.find_insert(RemoteHostSetItem(_subscriber));
                        sc->getSubscopes(_subscopes);
                        for (ScopeSetIter sc_set_it = _subscopes.begin(); sc_set_it != _subscopes.end(); sc_set_it++) {
                            notifySubscribers((*sc_set_it)._scpointer, subscribers);
                        }

                        /*then find all InformationItems for which the _subscriber is interested in*/
                        InformationItemSet _informationitems;
                        sc->findAllInformationItems(_informationitems);
                        /*then, for each one do the rendez-vous process*/
                        InformationItemSetIter pub_it;
                        for (pub_it = _informationitems.begin(); pub_it != _informationitems.end(); pub_it++) {
                            RemoteHostSet subscribers;
                            (*pub_it)._iipointer->findSubscribers(subscribers);
                            /*careful here...I have multiple fathers*/
                            ScopeSetIter fathersc_it;
                            for (fathersc_it = (*pub_it)._iipointer->fatherScopes.begin(); fathersc_it != (*pub_it)._iipointer->fatherScopes.end(); fathersc_it++) {
                                (*fathersc_it)._scpointer->findSubscribers(subscribers);
                            }
                            rendezvous((*pub_it)._iipointer, subscribers);
                        }
                    }
                } else {
                    click_chatter("LocalRV: strategies don't match....aborting subscription");
                }
            }
        } else {
            click_chatter("LocalRV: Cannot subscribe - father scope not exist!");
        }
    } else {
        click_chatter("LocalRV: Cannot subscribe to scope - a piece of info with the same path_id exists");
    }
}

void LocalRV::subscribe_info(RemoteHost *_subscriber, String &ID, String &prefixID, unsigned char &strategy) {
    InformationItem *pub;
    Scope *fatherScope;
    if ((prefixID.length() > 0) && (ID.length() == PURSUIT_ID_LEN)) {
        String fullID = prefixID + ID;
        /*the publisher advertises a piece (a single ID fragment) of info under a path that must exist*/
        fatherScope = scopeIndex.get(prefixID);
        if (fatherScope != scopeIndex.default_value()) {
            /*check if the InformationItem (with this specific ID) is already there...*/
            pub = pubIndex.get(fullID);
            if (pub == pubIndex.default_value()) {
                /*check if a scope with the same ID exists*/
                if (scopeIndex.find(fullID) == scopeIndex.end()) {
                    if (fatherScope->strategy == strategy) {
                        pub = new InformationItem(fatherScope->strategy, fatherScope);
                        pub->updateIDs(pubIndex, fullID.substring(fullID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN));
                        if (pub->updateSubscribers(fullID, _subscriber)) {
                            /*add the InformationItem to the subscriber's set*/
                            _subscriber->subscribedInformationItems.find_insert(StringSetItem(fullID));
                            click_chatter("LocalRV: added subscriber %s to (new) information item %s(%d)", _subscriber->remoteHostID.c_str(), pub->printID().c_str(), (int) strategy);
                        }
                    } else {
                        click_chatter("LocalRV: Error could not add subscription - strategy mismatch");
                    }
                } else {
                    click_chatter("LocalRV: Error - cannot subscribe to info - a scope with the same ID exists");
                }
            } else {
                if (fatherScope->strategy == strategy) {
                    if (pub->updateSubscribers(fullID, _subscriber)) {
                        /*add the scope to the publisher's set*/
                        _subscriber->subscribedInformationItems.find_insert(StringSetItem(fullID));
                        /*do the rendez-vous process*/
                        RemoteHostSet subscribers;
                        pub->findSubscribers(subscribers);
                        /*careful here...I have multiple fathers*/
                        ScopeSetIter fathersc_it;
                        for (fathersc_it = pub->fatherScopes.begin(); fathersc_it != pub->fatherScopes.end(); fathersc_it++) {
                            (*fathersc_it)._scpointer->findSubscribers(subscribers);
                        }
                        rendezvous(pub, subscribers);
                        click_chatter("LocalRV: added subscriber %s to information item %s(%d)", _subscriber->remoteHostID.c_str(), pub->printID().c_str(), (int) strategy);
                    }
                } else {
                    click_chatter("LocalRV: Error could not subscribe to InformationItem - strategy mismatch");
                }
            }
        } else {
            click_chatter("LocalRV: Error - Scope prefix %s doesn't exist", prefixID.quoted_hex().c_str());
        }
    }
}

void LocalRV::unsubscribe_scope(RemoteHost *_subscriber, String &ID, String &prefixID, unsigned char &strategy) {
    Scope *sc;
    Scope *fatherScope;
    String fullID = prefixID + ID;
    sc = scopeIndex.get(fullID);
    if (sc != scopeIndex.default_value()) {
        if (sc->strategy == strategy) {
            fatherScope = scopeIndex.get(prefixID);
            if (fatherScope != scopeIndex.default_value()) {
                /*not a root scope*/
                RemoteHostPair * pair = sc->ids.get(fullID);
                /*erase _subscriber (if it exists) (second in the pair) from the appropriate ID pair*/
                if (pair->second.find(_subscriber) != pair->second.end()) {
                    pair->second.erase(_subscriber);
                    _subscriber->subscribedScopes.erase(fullID);
                    /*find all pieces of info that are affected by this and do the rendez-vous*/
                    InformationItemSet _informationitems;
                    sc->findAllInformationItems(_informationitems);
                    /*then, for each one do the rendez-vous process*/
                    for (InformationItemSetIter pub_it = _informationitems.begin(); pub_it != _informationitems.end(); pub_it++) {
                        RemoteHostSet subscribers;
                        (*pub_it)._iipointer->findSubscribers(subscribers);
                        /*careful here...I have multiple fathers*/
                        for (ScopeSetIter fathersc_it = (*pub_it)._iipointer->fatherScopes.begin(); fathersc_it != (*pub_it)._iipointer->fatherScopes.end(); fathersc_it++) {
                            (*fathersc_it)._scpointer->findSubscribers(subscribers);
                        }
                        rendezvous((*pub_it)._iipointer, subscribers);
                    }
                    /*do not try to delete if there are subscopes or InformationItems under the scope*/
                    if ((sc->childrenScopes.size() == 0) && (sc->informationitems.size() == 0)) {
                        /*different approach is followed depending on the number of father scopes (NOT on the number of IDS)*/
                        if (sc->fatherScopes.size() == 1) {
                            //click_chatter("LocalRV: scope has a single father scope");
                            if (!sc->checkForOtherPubSub(fatherScope)) {
                                /*safe to delete Scope*/
                                fatherScope->childrenScopes.erase(sc);
                                click_chatter("LocalRV: deleted subscriber %s from (deleted) scope %s(%d)", _subscriber->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                                /*delete all IDs from scopeIndex*/
                                for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
                                    scopeIndex.erase((*it).first);
                                }
                                delete sc;
                            } else {
                                //click_chatter("LocalRV: deleted subscriber %s from scope %s(%d)", _subscriber->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                            }
                        } else {
                            //click_chatter("scope has many father scopes");
                            if (!sc->checkForOtherPubSub(fatherScope)) {
                                /*safe to delete scope (only this the specific branch)*/
                                fatherScope->childrenScopes.erase(sc);
                                sc->fatherScopes.erase(fatherScope);
                                click_chatter("LocalRV: deleted subscriber %s from (deleted) scope branch %s(%d)", _subscriber->remoteHostID.c_str(), fullID.quoted_hex().c_str(), (int) strategy);
                                /*delete all IDs from scopeIndex*/
                                String suffixID = (*sc->ids.begin()).first.substring((*sc->ids.begin()).first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                                for (IdsHashMapIter it = fatherScope->ids.begin(); it != fatherScope->ids.end(); it++) {
                                    delete sc->ids.get((*it).first + suffixID);
                                    sc->ids.erase((*it).first + suffixID);
                                    scopeIndex.erase((*it).first + suffixID);
                                }
                            } else {
                                click_chatter("LocalRV: deleted subscriber %s from scope branch %s(%d)", _subscriber->remoteHostID.c_str(), fullID.quoted_hex().c_str(), (int) strategy);
                            }
                        }
                    } else {
                        click_chatter("LocalRV: deleted subscriber %s from scope %s(%d)", _subscriber->remoteHostID.c_str(), fullID.quoted_hex().c_str(), (int) strategy);
                    }
                }
            } else {
                /*a ROOT scope*/
                RemoteHostPair * pair = sc->ids.get(fullID);
                /*erase _subscriber (if it exists) (second in the pair) from the appropriate ID pair*/
                if (pair->second.find(_subscriber) != pair->second.end()) {
                    pair->second.erase(_subscriber);
                    _subscriber->subscribedScopes.erase(fullID);
                    /*find all pieces of info that are affected by this and do the rendez-vous process*/
                    InformationItemSet _informationitems;
                    sc->findAllInformationItems(_informationitems);
                    /*then, for each one do the rendez-vous process*/
                    for (InformationItemSetIter pub_it = _informationitems.begin(); pub_it != _informationitems.end(); pub_it++) {
                        RemoteHostSet subscribers;
                        (*pub_it)._iipointer->findSubscribers(subscribers);
                        /*careful here...I have multiple fathers*/
                        ScopeSetIter fathersc_it;
                        for (fathersc_it = (*pub_it)._iipointer->fatherScopes.begin(); fathersc_it != (*pub_it)._iipointer->fatherScopes.end(); fathersc_it++) {
                            (*fathersc_it)._scpointer->findSubscribers(subscribers);
                        }
                        rendezvous(pub_it.get()->_iipointer, subscribers);
                    }
                    /*do not try to delete after unsubscribing if there are subscopes or informationitems under the scope*/
                    if ((sc->childrenScopes.size() == 0) && (sc->informationitems.size() == 0)) {
                        //click_chatter("scope is a root scope");
                        if (!sc->checkForOtherPubSub(NULL)) {
                            click_chatter("LocalRV: deleted subscriber %s from (deleted) scope %s(%d)", _subscriber->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                            /*delete all IDs from scopeIndex*/
                            for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
                                scopeIndex.erase((*it).first);
                            }
                            delete sc;
                        } else {
                            click_chatter("LocalRV: deleted subscriber %s from scope %s(%d)", _subscriber->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                        }
                    } else {
                        click_chatter("LocalRV: deleted subscriber %s from scope %s(%d)", _subscriber->remoteHostID.c_str(), sc->printID().c_str(), (int) strategy);
                    }
                }
            }
        } else {
            click_chatter("LocalRV: Cannot Unsubscribe from scope %s..strategy mismatch", fullID.quoted_hex().c_str());
        }
    } else {
        click_chatter("LocalRV: Scope %s does not exist...Unsubscribe from what?", fullID.quoted_hex().c_str());
    }
}

void LocalRV::unsubscribe_info(RemoteHost *_subscriber, String &ID, String &prefixID, unsigned char &strategy) {
    InformationItem *pub;
    Scope *fatherScope;
    String fullID = prefixID + ID;
    pub = pubIndex.get(fullID);
    fatherScope = scopeIndex.get(prefixID);
    if (pub != pubIndex.default_value()) {
        if (fatherScope->strategy == strategy) {
            RemoteHostPair * pair = pub->ids.get(fullID);
            /*erase _subscriber (if it exists) (second in the pair) from the appropriate ID pair*/
            if (pair->second.find(_subscriber) != pair->second.end()) {
                pair->second.erase(_subscriber);
                _subscriber->subscribedInformationItems.erase(fullID);
                /*do the rendez-vous if there are any left publishers and subscribers*/
                RemoteHostSet subscribers;
                pub->findSubscribers(subscribers);
                /*careful here...I have multiple fathers*/
                ScopeSetIter fathersc_it;
                for (fathersc_it = pub->fatherScopes.begin(); fathersc_it != pub->fatherScopes.end(); fathersc_it++) {
                    (*fathersc_it)._scpointer->findSubscribers(subscribers);
                }
                rendezvous(pub, subscribers);
                /*different approach is followed depending on the number of father scopes (NOT on the number of IDS)*/
                if (pub->fatherScopes.size() == 1) {
                    //click_chatter("LocalRV: info has a single father scope");
                    if (!pub->checkForOtherPubSub(fatherScope)) {
                        /*safe to delete InformationItem*/
                        fatherScope->informationitems.erase(pub);
                        click_chatter("LocalRV: deleted subscriber %s from (deleted) information item %s(%d)", _subscriber->remoteHostID.c_str(), pub->printID().c_str(), (int) strategy);
                        /*delete all IDs from pubIndex*/
                        for (IdsHashMapIter it = pub->ids.begin(); it != pub->ids.end(); it++) {
                            pubIndex.erase((*it).first);
                        }
                        delete pub;
                    } else {
                        //click_chatter("LocalRV: deleted subscriber %s from information item %s(%d)", _subscriber->remoteHostID.c_str(), pub->printID().c_str(), (int) strategy);
                    }
                } else {
                    //click_chatter("info has many father scopes");
                    if (!pub->checkForOtherPubSub(fatherScope)) {
                        /*safe to delete InformationItem*/
                        fatherScope->informationitems.erase(pub);
                        pub->fatherScopes.erase(fatherScope);
                        click_chatter("LocalRV: deleted subscriber %s from (deleted) information item branch %s(%d)", _subscriber->remoteHostID.c_str(), fullID.quoted_hex().c_str(), (int) strategy);
                        /*delete all IDs from pubIndex*/
                        String suffixID = (*pub->ids.begin()).first.substring((*pub->ids.begin()).first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                        for (IdsHashMapIter it = fatherScope->ids.begin(); it != fatherScope->ids.end(); it++) {
                            /*since the pub is not deleted, manually delete this pair*/
                            delete pub->ids.get((*it).first + suffixID);
                            pub->ids.erase((*it).first + suffixID);
                            pubIndex.erase((*it).first + suffixID);
                        }
                    } else {
                        click_chatter("LocalRV: deleted subscriber %s from information item branch %s(%d)", _subscriber->remoteHostID.c_str(), fullID.quoted_hex().c_str(), (int) strategy);
                    }
                }
            }
        } else {
            click_chatter("LocalRV:Cannot Unsubscribe from %s..strategy mismatch", pub->printID().c_str());
        }
    } else {
        click_chatter("LocalRV:InformationItem %s does not exist...Unsubscribes from what", fullID.quoted_hex().c_str());
    }
}

/*everything should be sent to the local proxy using the blackadder API*/
void LocalRV::requestTMAssistanceForRendezvous(InformationItem *pub, RemoteHostSet &_publishers, RemoteHostSet &_subscribers, IdsHashMap &IDs) {
    /*Publish a request to the TM*/
    int packet_len;
    WritablePacket *p;
    /********FOR THE API*********/
    unsigned char typeForAPI = PUBLISH_DATA;
    unsigned char IDLenForAPI = 2 * PURSUIT_ID_LEN / PURSUIT_ID_LEN;
    unsigned char strategy = PUBLISH_NOW;
    /****************************/
    unsigned char request_type = MATCH_PUB_SUBS;
    unsigned char no_publishers = _publishers.size();
    int publisher_index = 0;
    unsigned char no_subscribers = _subscribers.size();
    int subscriber_index = 0;
    unsigned char no_ids = IDs.size();
    unsigned char IDs_total_bytes = 0;
    int ids_index = 0;
    for (IdsHashMapIter iter = pub->ids.begin(); iter != pub->ids.end(); iter++) {
        IDs_total_bytes += (*iter).first.length();
    }
    /*allocate the packet*/
    packet_len = /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN/*END OF API*/\
            /*PAYLOAD*/ + sizeof (request_type) + sizeof (no_publishers) /*sizeof(numberOfPubs)*/ + _publishers.size() * NODEID_LEN + sizeof (no_subscribers) /*sizeof(numberOfSubs)*/ + _subscribers.size() * NODEID_LEN \
                    + sizeof (no_ids) /*sizeof(numberOFIDs)*/ + pub->ids.size() * sizeof (unsigned char) +IDs_total_bytes;
    p = Packet::make(50, NULL, packet_len, 0);
    /*For the API*/
    memcpy(p->data(), &typeForAPI, sizeof (typeForAPI));
    memcpy(p->data() + sizeof (typeForAPI), &IDLenForAPI, sizeof (IDLenForAPI));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI), gc->nodeTMScope.c_str(), gc->nodeTMScope.length());
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length(), &strategy, sizeof (strategy));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy), gc->TMFID._data, FID_LEN);
    /*Put the payload*/
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN, &request_type, sizeof (request_type));
    /*put the publisher IDs*/
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type), &no_publishers, sizeof (no_publishers));
    for (RemoteHostSetIter iter = _publishers.begin(); iter != _publishers.end(); iter++) {
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type) + sizeof (no_publishers) + publisher_index, (*iter)._rhpointer->remoteHostID.c_str(), (*iter)._rhpointer->remoteHostID.length());
        publisher_index += (*iter)._rhpointer->remoteHostID.length();
    }
    /*put the subscriber IDs*/
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type) + sizeof (no_publishers) + publisher_index, &no_subscribers, sizeof (no_subscribers));
    for (RemoteHostSetIter iter = _subscribers.begin(); iter != _subscribers.end(); iter++) {
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type) + sizeof (no_publishers) + publisher_index + sizeof (no_subscribers) + subscriber_index, (*iter)._rhpointer->remoteHostID.c_str(), (*iter)._rhpointer->remoteHostID.length());
        subscriber_index += (*iter)._rhpointer->remoteHostID.length();
    }
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type) + sizeof (no_publishers) + publisher_index + sizeof (no_subscribers) + subscriber_index, &no_ids, sizeof (no_ids));
    /*put the pathIDs of the information item*/
    for (IdsHashMapIter iter = IDs.begin(); iter != IDs.end(); iter++) {
        unsigned char IDLength = (unsigned char) (*iter).first.length() / PURSUIT_ID_LEN;
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type) + sizeof (no_publishers) + publisher_index + sizeof (no_subscribers) + subscriber_index + sizeof (no_ids) + ids_index, &IDLength, sizeof (IDLength));
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type) + sizeof (no_publishers) + publisher_index + sizeof (no_subscribers) + subscriber_index + sizeof (no_ids) + ids_index + sizeof (IDLength), (*iter).first.c_str(), (*iter).first.length());
        ids_index += sizeof (IDLength) + (*iter).first.length();
    }
    p->set_anno_u32(0, RV_ELEMENT);
    output(0).push(p);
}

void LocalRV::requestTMAssistanceForNotifyingSubscribers(Scope *sc, unsigned char request_type, RemoteHostSet &_subscribers, IdsHashMap &IDs) {
    /*Publish a request to the TM*/
    int packet_len;
    WritablePacket *p;
    /********FOR THE API*********/
    unsigned char typeForAPI = PUBLISH_DATA;
    unsigned char IDLenForAPI = 2 * PURSUIT_ID_LEN / PURSUIT_ID_LEN;
    unsigned char strategy = PUBLISH_NOW;
    /****************************/
    unsigned char no_subscribers = _subscribers.size();
    int subscriber_index = 0;
    unsigned char no_ids = IDs.size();
    unsigned char IDs_total_bytes = 0;
    int ids_index = 0;
    for (IdsHashMapIter iter = sc->ids.begin(); iter != sc->ids.end(); iter++) {
        IDs_total_bytes += (*iter).first.length();
    }
    /*allocate the packet*/
    packet_len = /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN/*END OF API*/\
            /*PAYLOAD*/ + sizeof (request_type) + sizeof (no_subscribers) /*sizeof(numberOfSubs)*/ + _subscribers.size() * NODEID_LEN \
                    + sizeof (no_ids) /*sizeof(numberOfpathIDs)*/ + sc->ids.size() * sizeof (unsigned char) +IDs_total_bytes;
    p = Packet::make(50, NULL, packet_len, 0);
    /*For the API*/
    memcpy(p->data(), &typeForAPI, sizeof (typeForAPI));
    memcpy(p->data() + sizeof (typeForAPI), &IDLenForAPI, sizeof (IDLenForAPI));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI), gc->nodeTMScope.c_str(), gc->nodeTMScope.length());
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length(), &strategy, sizeof (strategy));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy), gc->TMFID._data, FID_LEN);
    /*Put the payload*/
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN, &request_type, sizeof (request_type));
    /*put the subscriber IDs*/
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type), &no_subscribers, sizeof (no_subscribers));
    for (RemoteHostSetIter iter = _subscribers.begin(); iter != _subscribers.end(); iter++) {
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type) + sizeof (no_subscribers) + subscriber_index, (*iter)._rhpointer->remoteHostID.c_str(), (*iter)._rhpointer->remoteHostID.length());
        subscriber_index += (*iter)._rhpointer->remoteHostID.length();
    }
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type) + sizeof (no_subscribers) + subscriber_index, &no_ids, sizeof (no_ids));
    /*put the pathIDs of the information item*/
    for (IdsHashMapIter iter = IDs.begin(); iter != IDs.end(); iter++) {
        unsigned char IDLength = (unsigned char) (*iter).first.length() / PURSUIT_ID_LEN;
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type) + sizeof (no_subscribers) + subscriber_index + sizeof (no_ids) + ids_index, &IDLength, sizeof (IDLength));
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type) + sizeof (no_subscribers) + subscriber_index + sizeof (no_ids) + ids_index + sizeof (IDLength), (*iter).first.c_str(), (*iter).first.length());
        ids_index += sizeof (IDLength) + (*iter).first.length();
    }
    p->set_anno_u32(0, RV_ELEMENT);
    output(0).push(p);
}

/*LINK_LOCAL STILL DOES NOT PROPERLY*/
void LocalRV::rendezvous(InformationItem *pub, RemoteHostSet &_subscribers) {
    RemoteHostSet _publishers;
    RemoteHostSetIter _pub_it;
    RemoteHost *_publisher;
    RemoteHost *_subscriber;
    Neighbour *neigh;
    pub->findPublishers(_publishers);
    /*I have a publication..it can have zero, one or many publishers..(check all ids)*/
    if (_publishers.size() > 0) {
        if (pub->strategy == NODE_LOCAL) {
            /*all publishers and subscribers should be running locally*/
            if (_subscribers.size() > 0) {
                notifyLocalPublisher(pub, gc->iLID);
            } else {
                notifyLocalPublisher(pub, NULL);
            }
        } else if (pub->strategy == LINK_LOCAL) {
            if (_subscribers.size() > 0) {
                /*get the first publisher and send it an FID..all the others will get NULL FID*/
                _publisher = (*_publishers.begin())._rhpointer;
                BABitvector *FID = new BABitvector(FID_LEN * 8);
                for (RemoteHostSetIter iter = _subscribers.begin(); iter != _subscribers.end(); iter++) {
                    _subscriber = (*iter)._rhpointer;
                    if (_subscriber == localProxy) {
                        *FID = (*FID) | (*gc->iLID);
                    } else {
                        neigh = neighbours.get(_subscriber->remoteHostID);
                        BABitvector *temp_FID = neigh->getFID();
                        *FID = (*FID) | (*temp_FID);
                        delete temp_FID;
                    }
                }
                if (_publisher == localProxy) {
                    notifyLocalPublisher(pub, FID);
                } else {
                    neigh = neighbours.get(_publisher->remoteHostID);
                    BABitvector *FID_to_publisher = neigh->getFID();
                    notifyRemotePublisher(_publisher, pub, FID, FID_to_publisher);
                }
                /*for all other publishers send NULL FID*/
                RemoteHostSetIter _pub_it = _publishers.begin();
                _pub_it++;
                for (; _pub_it != _publishers.end(); _pub_it++) {
                    _publisher = (*_pub_it)._rhpointer;
                    if (_publisher == localProxy) {
                        notifyLocalPublisher(pub, NULL);
                    } else {
                        neigh = neighbours.get(_publisher->remoteHostID);
                        BABitvector *FID_to_publisher = neigh->getFID();
                        notifyRemotePublisher(_publisher, pub, NULL, FID_to_publisher);
                    }
                }
            } else {
                for (_pub_it = _publishers.begin(); _pub_it != _publishers.end(); _pub_it++) {
                    _publisher = (*_pub_it)._rhpointer;
                    if (_publisher == localProxy) {
                        notifyLocalPublisher(pub, NULL);
                    } else {
                        neigh = neighbours.get(_publisher->remoteHostID);
                        BABitvector *FID_to_publisher = neigh->getFID();
                        notifyRemotePublisher(_publisher, pub, NULL, FID_to_publisher);
                    }
                }
            }
        } else if (pub->strategy == DOMAIN_LOCAL) {
            requestTMAssistanceForRendezvous(pub, _publishers, _subscribers, pub->ids);
        }
    }
}

void LocalRV::notifyLocalPublisher(InformationItem *pub, BABitvector *FID) {
    WritablePacket *p;
    /********FOR THE API*********/
    unsigned char typeForAPI = PUBLISH_DATA;
    unsigned char IDLenForAPI = gc->notificationIID.length() / PURSUIT_ID_LEN;
    unsigned char strategy = PUBLISH_NOW;
    /****************************/
    int index, totalIDsLength = 0;
    unsigned char type, numberOfIDs;
    /*for the "header"*/
    numberOfIDs = (unsigned char) pub->ids.size();
    for (IdsHashMapIter it = pub->ids.begin(); it != pub->ids.end(); it++) {
        totalIDsLength = totalIDsLength + (*it).first.length();
    }
    /**********************************************/
    if (FID != NULL) {
        p = Packet::make(30, NULL, /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN/*END OF API*/ + sizeof (type) + sizeof (numberOfIDs)+((int) numberOfIDs) * sizeof (unsigned char) +totalIDsLength + FID_LEN, 0);
        type = START_PUBLISH;
        //click_chatter("LocalRV: locally pushing NEW FID");
    } else {
        p = Packet::make(30, NULL, /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN/*END OF API*/ + sizeof (type) + sizeof (numberOfIDs)+((int) numberOfIDs) * sizeof (unsigned char) +totalIDsLength, 0);
        type = STOP_PUBLISH;
        //click_chatter("LocalRV: locally pushing NULL FID");
    }

    /**********************************************/
    memcpy(p->data(), &typeForAPI, sizeof (typeForAPI));
    memcpy(p->data() + sizeof (typeForAPI), &IDLenForAPI, sizeof (IDLenForAPI));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI), gc->notificationIID.c_str(), gc->notificationIID.length());
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length(), &strategy, sizeof (strategy));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy), gc->iLID->_data, FID_LEN);
    /**********************************************/
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN, &type, sizeof (type));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN + sizeof (type), &numberOfIDs, sizeof (numberOfIDs));
    index = 0;
    IdsHashMapIter it = pub->ids.begin();
    for (int i = 0; i < (int) numberOfIDs; i++) {
        unsigned char IDLength = (unsigned char) it.key().length() / PURSUIT_ID_LEN;
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index, &IDLength, sizeof (IDLength));
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index + sizeof (IDLength), (*it).first.c_str(), IDLength * PURSUIT_ID_LEN);
        index = index + sizeof (IDLength) + IDLength * PURSUIT_ID_LEN;
        it++;
    }
    if (FID != NULL) {
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index, FID->_data, FID_LEN);
    }
    p->set_anno_u32(0, RV_ELEMENT);
    output(0).push(p);
}

/*IF we get to this method, then for some reason (e.g. link_local strategy) the FIDToPublisher is resolved*/
void LocalRV::notifyRemotePublisher(RemoteHost *remotePublisher, InformationItem *pub, BABitvector *FID, BABitvector *FIDToPublisher) {
    WritablePacket *p;
    /********FOR THE API*********/
    unsigned char typeForAPI = PUBLISH_DATA;
    unsigned char IDLenForAPI = 2 * PURSUIT_ID_LEN / PURSUIT_ID_LEN;
    unsigned char strategy = PUBLISH_NOW;
    /****************************/
    int index, totalIDsLength = 0;
    unsigned char type, numberOfIDs;
    /*for the "header"*/
    numberOfIDs = (unsigned char) pub->ids.size();
    for (IdsHashMapIter it = pub->ids.begin(); it != pub->ids.end(); it++) {
        totalIDsLength = totalIDsLength + (*it).first.length();
    }
    /**********************************************/
    if (FID != NULL) {
        p = Packet::make(30, NULL, /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN/*END OF API*/ + sizeof (type) + sizeof (numberOfIDs)+((int) numberOfIDs) * sizeof (unsigned char) +totalIDsLength + FID_LEN, 0);
        type = START_PUBLISH;
        //click_chatter("LocalRV: locally pushing NEW FID");
    } else {
        p = Packet::make(30, NULL, /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN/*END OF API*/ + sizeof (type) + sizeof (numberOfIDs)+((int) numberOfIDs) * sizeof (unsigned char) +totalIDsLength, 0);
        type = STOP_PUBLISH;
        //click_chatter("LocalRV: locally pushing NULL FID");
    }
    /**********************************************/
    memcpy(p->data(), &typeForAPI, sizeof (typeForAPI));
    memcpy(p->data() + sizeof (typeForAPI), &IDLenForAPI, sizeof (IDLenForAPI));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI), gc->notificationIID.substring(0, PURSUIT_ID_LEN).c_str(), PURSUIT_ID_LEN);
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + PURSUIT_ID_LEN, remotePublisher->remoteHostID.c_str(), PURSUIT_ID_LEN);
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN, &strategy, sizeof (strategy));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy), FIDToPublisher->_data, FID_LEN);
    /**********************************************/
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN, &type, sizeof (type));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN + sizeof (type), &numberOfIDs, sizeof (numberOfIDs));
    index = 0;
    IdsHashMapIter it = pub->ids.begin();
    for (int i = 0; i < (int) numberOfIDs; i++) {
        unsigned char IDLength = (unsigned char) it.key().length() / PURSUIT_ID_LEN;
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index, &IDLength, sizeof (IDLength));
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index + sizeof (IDLength), (*it).first.c_str(), IDLength * PURSUIT_ID_LEN);
        index = index + sizeof (IDLength) + IDLength * PURSUIT_ID_LEN;
        it++;
    }
    if (FID != NULL) {
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index, FID->_data, FID_LEN);
        delete FID;
    }
    p->set_anno_u32(0, RV_ELEMENT);
    output(0).push(p);
    /*i can delete that since it has been previously allocated and calculated*/
    delete FIDToPublisher;
}

/*LINK_LOCAL STILL DOES NOT PROPERLY*/
void LocalRV::notifySubscribers(Scope *sc, RemoteHostSet &subscribers) {
    BABitvector *FIDToSubscriber;
    Neighbour *neigh;
    RemoteHost *_subscriber;
    if (sc->strategy == NODE_LOCAL) {
        /*In this case all subscribers are running locally*/
        if (subscribers.find(localProxy) != subscribers.end()) {
            notifyLocalSubscriber(sc);
        }
    } else if (sc->strategy == LINK_LOCAL) {
        /*I can have local subscribers and physical neighbors*/
        for (RemoteHostSetIter subscribers_it = subscribers.begin(); subscribers_it != subscribers.end(); subscribers_it++) {
            _subscriber = (*subscribers_it)._rhpointer;
            if (_subscriber == localProxy) {
                notifyLocalSubscriber(sc);
            } else {
                neigh = neighbours.get(_subscriber->remoteHostID);
                FIDToSubscriber = neigh->getFID();
                notifyRemoteSubscriber(sc, _subscriber, FIDToSubscriber);
            }
        }
    } else {
        if (subscribers.size() > 0) {
            requestTMAssistanceForNotifyingSubscribers(sc, (unsigned char) SCOPE_PUBLISHED, subscribers, sc->ids);
        }
    }
}

/*LINK_LOCAL STILL DOES NOT PROPERLY*/
/*exclude the existingPrefixID*/
void LocalRV::notifySubscribers(Scope *sc, String &existingPrefixID, RemoteHostSet &subscribers) {
    BABitvector *FIDToSubscriber;
    Neighbour *neigh;
    RemoteHost *_subscriber;
    if (sc->strategy == NODE_LOCAL) {
        /*In this case all subscribers are local apps*/
        if (subscribers.find(localProxy) != subscribers.end()) {
            notifyLocalSubscriber(sc, existingPrefixID);
        }
    } else if (sc->strategy == LINK_LOCAL) {
        /*I can have local subscribers and physical neighbors*/
        for (RemoteHostSetIter subscribers_it = subscribers.begin(); subscribers_it != subscribers.end(); subscribers_it++) {
            _subscriber = (*subscribers_it)._rhpointer;
            if (_subscriber == localProxy) {
                notifyLocalSubscriber(sc, existingPrefixID);
            } else {
                neigh = neighbours.get(_subscriber->remoteHostID);
                FIDToSubscriber = neigh->getFID();
                notifyRemoteSubscriber(sc, existingPrefixID, _subscriber, FIDToSubscriber);
            }
        }
    } else {
        /*I will create a copy here..I will put only the IDs in the hashtable...that's what I need here*/
        IdsHashMap IDs;
        if (subscribers.size() > 0) {
            for (IdsHashMapIter iter = sc->ids.begin(); iter != sc->ids.end(); iter++) {
                if (existingPrefixID.compare((*iter).first.substring(0, existingPrefixID.length())) != 0) {
                    IDs.set((*iter).first, NULL);
                }
            }
            /*I will use the copy of the IdsHashMap that does not contain IDs starting with existingPrefixID which must be excluded*/
            requestTMAssistanceForNotifyingSubscribers(sc, (unsigned char) SCOPE_PUBLISHED, subscribers, IDs);
        }
    }
}

void LocalRV::notifyLocalSubscriber(Scope *sc) {
    WritablePacket *p;
    /********FOR THE API*********/
    unsigned char typeForAPI = PUBLISH_DATA;
    unsigned char IDLenForAPI = gc->notificationIID.length() / PURSUIT_ID_LEN;
    unsigned char strategy = PUBLISH_NOW;
    /****************************/
    int index, totalIDsLength = 0;
    unsigned char type, numberOfIDs;
    type = SCOPE_PUBLISHED;
    numberOfIDs = (unsigned char) sc->ids.size();
    for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
        totalIDsLength = totalIDsLength + (*it).first.length();
    }
    /**********************************************/
    p = Packet::make(30, NULL, /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN/*END OF API*/ + sizeof (type) + sizeof (numberOfIDs)+((int) numberOfIDs) * sizeof (unsigned char) +totalIDsLength, 0);
    memcpy(p->data(), &typeForAPI, sizeof (typeForAPI));
    memcpy(p->data() + sizeof (typeForAPI), &IDLenForAPI, sizeof (IDLenForAPI));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI), gc->notificationIID.c_str(), gc->notificationIID.length());
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length(), &strategy, sizeof (strategy));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy), gc->iLID->_data, FID_LEN);
    /***********************************************/
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN, &type, sizeof (type));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN + sizeof (type), &numberOfIDs, sizeof (numberOfIDs));
    index = 0;
    IdsHashMapIter it = sc->ids.begin();
    for (int i = 0; i < (int) numberOfIDs; i++) {
        unsigned char IDLength = (unsigned char) (*it).first.length() / PURSUIT_ID_LEN;
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index, &IDLength, sizeof (IDLength));
        memcpy(p->data() + + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index + sizeof (IDLength), (*it).first.c_str(), IDLength * PURSUIT_ID_LEN);
        index = index + sizeof (IDLength) + IDLength * PURSUIT_ID_LEN;
        it++;
    }
    p->set_anno_u32(0, RV_ELEMENT);
    output(0).push(p);
}

/*exclude the existingPrefixID*/
void LocalRV::notifyLocalSubscriber(Scope *sc, String &existingPrefixID) {

    WritablePacket *p;
    /********FOR THE API*********/
    unsigned char typeForAPI = PUBLISH_DATA;
    unsigned char IDLenForAPI = gc->notificationIID.length() / PURSUIT_ID_LEN;
    unsigned char strategy = PUBLISH_NOW;
    /****************************/
    int index, totalIDsLength = 0;
    unsigned char type, numberOfIDs;
    type = SCOPE_PUBLISHED;
    numberOfIDs = 0;
    for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
        if (existingPrefixID.compare((*it).first.substring(0, existingPrefixID.length())) != 0) {
            totalIDsLength = totalIDsLength + (*it).first.length();
            numberOfIDs++;
        }
    }
    /**********************************************/
    p = Packet::make(30, NULL, /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN/*END OF API*/ + sizeof (type) + sizeof (numberOfIDs)+((int) numberOfIDs) * sizeof (unsigned char) +totalIDsLength, 0);
    memcpy(p->data(), &typeForAPI, sizeof (typeForAPI));
    memcpy(p->data() + sizeof (typeForAPI), &IDLenForAPI, sizeof (IDLenForAPI));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI), gc->notificationIID.c_str(), gc->notificationIID.length());
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length(), &strategy, sizeof (strategy));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy), gc->iLID->_data, FID_LEN);
    /***********************************************/

    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN, &type, sizeof (type));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN + sizeof (type), &numberOfIDs, sizeof (numberOfIDs));
    index = 0;
    for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
        if (existingPrefixID.compare((*it).first.substring(0, existingPrefixID.length())) != 0) {
            unsigned char IDLength = (unsigned char) (*it).first.length() / PURSUIT_ID_LEN;
            memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index, &IDLength, sizeof (IDLength));
            memcpy(p->data() + + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->notificationIID.length() + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index + sizeof (IDLength), (*it).first.c_str(), IDLength * PURSUIT_ID_LEN);
            index = index + sizeof (IDLength) + IDLength * PURSUIT_ID_LEN;
        }
    }
    p->set_anno_u32(0, RV_ELEMENT);
    output(0).push(p);
}

/*IF we got to this method, then for some reason (e.g. link_local strategy) the FIDToSubscriber is resolved*/
void LocalRV::notifyRemoteSubscriber(Scope *sc, RemoteHost *subscriber, BABitvector *FIDToSubscriber) {
    WritablePacket *p;
    /********FOR THE API*********/
    unsigned char typeForAPI = PUBLISH_DATA;
    unsigned char IDLenForAPI = 2 * PURSUIT_ID_LEN;
    unsigned char strategy = PUBLISH_NOW;
    /****************************/
    int index, totalIDsLength = 0;
    unsigned char type, numberOfIDs;
    type = SCOPE_PUBLISHED;
    numberOfIDs = (unsigned char) sc->ids.size();
    for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
        totalIDsLength = totalIDsLength + (*it).first.length();
    }
    /**********************************************/
    p = Packet::make(30, NULL, /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN/*END OF API*/ + sizeof (type) + sizeof (numberOfIDs)+((int) numberOfIDs) * sizeof (unsigned char) +totalIDsLength, 0);
    memcpy(p->data(), &typeForAPI, sizeof (typeForAPI));
    memcpy(p->data() + sizeof (typeForAPI), &IDLenForAPI, sizeof (IDLenForAPI));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI), gc->notificationIID.substring(0, PURSUIT_ID_LEN).c_str(), PURSUIT_ID_LEN);
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + PURSUIT_ID_LEN, subscriber->remoteHostID.c_str(), PURSUIT_ID_LEN);
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN, &strategy, sizeof (strategy));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy), FIDToSubscriber->_data, FID_LEN);
    /***********************************************/

    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN, &type, sizeof (type));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN + sizeof (type), &numberOfIDs, sizeof (numberOfIDs));
    index = 0;
    IdsHashMapIter it = sc->ids.begin();
    for (int i = 0; i < (int) numberOfIDs; i++) {
        unsigned char IDLength = (unsigned char) (*it).first.length() / PURSUIT_ID_LEN;
        memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index, &IDLength, sizeof (IDLength));
        memcpy(p->data() + + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index + sizeof (IDLength), (*it).first.c_str(), IDLength * PURSUIT_ID_LEN);
        index = index + sizeof (IDLength) + IDLength * PURSUIT_ID_LEN;
        it++;
    }
    p->set_anno_u32(0, RV_ELEMENT);
    output(0).push(p);
    delete FIDToSubscriber;
}

/*IF we got to this method, then for some reason (e.g. link_local strategy) the FIDToSubscriber is resolved
exclude the existingPrefixID*/
void LocalRV::notifyRemoteSubscriber(Scope *sc, String &existingPrefixID, RemoteHost *subscriber, BABitvector *FIDToSubscriber) {
    WritablePacket *p;
    /********FOR THE API*********/
    unsigned char typeForAPI = PUBLISH_DATA;
    unsigned char IDLenForAPI = 2 * PURSUIT_ID_LEN;
    unsigned char strategy = PUBLISH_NOW;
    /****************************/
    int index, totalIDsLength = 0;
    unsigned char type, numberOfIDs;
    type = SCOPE_PUBLISHED;
    numberOfIDs = 0;
    for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
        if (existingPrefixID.compare((*it).first.substring(0, existingPrefixID.length())) != 0) {
            totalIDsLength = totalIDsLength + (*it).first.length();
            numberOfIDs++;
        }
    }
    p = Packet::make(30, NULL, /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN/*END OF API*/ + sizeof (type) + sizeof (numberOfIDs)+((int) numberOfIDs) * sizeof (unsigned char) +totalIDsLength, 0);
    /**********************************************/
    memcpy(p->data(), &typeForAPI, sizeof (typeForAPI));
    memcpy(p->data() + sizeof (typeForAPI), &IDLenForAPI, sizeof (IDLenForAPI));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI), gc->notificationIID.substring(0, PURSUIT_ID_LEN).c_str(), PURSUIT_ID_LEN);
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + PURSUIT_ID_LEN, subscriber->remoteHostID.c_str(), PURSUIT_ID_LEN);
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN, &strategy, sizeof (strategy));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy), FIDToSubscriber->_data, FID_LEN);
    /***********************************************/

    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN, &type, sizeof (type));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN + sizeof (type), &numberOfIDs, sizeof (numberOfIDs));
    index = 0;
    for (IdsHashMapIter it = sc->ids.begin(); it != sc->ids.end(); it++) {
        if (existingPrefixID.compare((*it).first.substring(0, existingPrefixID.length())) != 0) {
            unsigned char IDLength = (unsigned char) (*it).first.length() / PURSUIT_ID_LEN;
            memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index, &IDLength, sizeof (IDLength));
            memcpy(p->data() + + sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN + sizeof (type) + sizeof (numberOfIDs) + index + sizeof (IDLength), (*it).first.c_str(), IDLength * PURSUIT_ID_LEN);
            index = index + sizeof (IDLength) + IDLength * PURSUIT_ID_LEN;
        }
    }
    p->set_anno_u32(0, RV_ELEMENT);
    output(0).push(p);
    delete FIDToSubscriber;
}

RemoteHost * LocalRV::checkRemoteHost(String & nodeID) {
    RemoteHost *_remotehost = NULL;
    /*check if the _remotehost exists*/
    _remotehost = pub_sub_Index.get(nodeID);
    if (_remotehost == pub_sub_Index.default_value()) {
        return NULL;
    }
    return _remotehost;
}

RemoteHost * LocalRV::createRemoteHost(String & nodeID) {
    RemoteHost *_remotehost = NULL;
    _remotehost = pub_sub_Index.get(nodeID);
    if (_remotehost == pub_sub_Index.default_value()) {
        /*create a new _remotehost*/
        _remotehost = new RemoteHost(nodeID);
        pub_sub_Index.set(nodeID, _remotehost);
    }
    return _remotehost;
}

Neighbour::Neighbour(String _label, String _LID, String _iLID) {
    label = _label;
    LID = BABitvector(FID_LEN * 8);
    for (int i = 0; i < _LID.length(); i++) {
        if (_LID.at(i) == '1') {
            LID[LID.size() - i - 1] = true;
        }
    }
    iLID = BABitvector(FID_LEN * 8);
    for (int i = 0; i < _iLID.length(); i++) {
        if (_iLID.at(i) == '1') {
            iLID[iLID.size() - i - 1] = true;
        }
    }
}

BABitvector *Neighbour::getFID() {
    BABitvector *result = new BABitvector(FID_LEN * 8);
    *result = LID | iLID;
    return result;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(LocalRV)
