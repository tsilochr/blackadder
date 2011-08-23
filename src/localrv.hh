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

#ifndef CLICK_LOCALRV_HH
#define CLICK_LOCALRV_HH

#include "globalconf.hh"
#include "ba_bitvector.hh"
#include "informationitem.hh"
#include "scope.hh"
#include "remotehost.hh"
#include "common.hh"

CLICK_DECLS

class RemoteHost;
class Scope;
class InformationItem;

class Neighbour {
public:
    String label;
    BABitvector LID;
    BABitvector iLID;
    Neighbour(String _label, String _LID, String _iLID);
    BABitvector *getFID();
};

class LocalRV : public Element {
public:
    LocalRV();
    ~LocalRV();

    const char *class_name() const {return "LocalRV";}

    const char *port_count() const {
        return "-/-";
    }

    const char *processing() const {
        return PUSH;
    }
    int configure(Vector<String>&, ErrorHandler*);

    int configure_phase() const {
        return 201;
    }
    int initialize(ErrorHandler *errh);
    void cleanup(CleanupStage stage);
    void push(int, Packet *);

    void publish_scope(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy);
    void publish_root_scope(RemoteHost *_publisher, String &ID, unsigned char &strategy);
    void publish_inner_scope(RemoteHost *_publisher, String &id, String &prefix_id, unsigned char &strategy);
    void republish_inner_scope(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy);
    void publish_info(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy);
    void advertise_info(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy);
    void readvertise_info(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy);
    void unpublish_scope(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy);
    void unpublish_info(RemoteHost *_publisher, String &ID, String &prefixID, unsigned char &strategy);
    void subscribe_scope(RemoteHost *_subscriber, String &ID, String &prefixID, unsigned char &strategy);
    void subscribe_root_scope(RemoteHost *_subscriber, String &ID, unsigned char &strategy);
    void subscribe_inner_scope(RemoteHost *_subscriber, String &ID, String &prefixID, unsigned char &strategy);
    void subscribe_info(RemoteHost *_subscriber, String &ID, String &prefixID, unsigned char &strategy);
    void unsubscribe_scope(RemoteHost *_subscriber, String &ID, String &prefixID, unsigned char &strategy);
    void unsubscribe_info(RemoteHost *_subscriber, String &ID, String &prefixID, unsigned char &strategy);
    void publish(String id, String data);
    /**************************************************************************************************************************************/
    RemoteHost *createRemoteHost(String & nodeID);
    RemoteHost *checkRemoteHost(String & nodeID);
    /**************************************************************************************************************************************/
    void notifySubscribers(Scope *sc, RemoteHostSet &subscribers);
    void notifyLocalSubscriber(Scope *sc);
    void notifyRemoteSubscriber(Scope *sc, RemoteHost *subscriber, BABitvector *FIDToSubscriber);
    void notifySubscribers(Scope *sc, String &existingPrefixID, RemoteHostSet &subscribers);
    void notifyLocalSubscriber(Scope *sc, String &existingPrefixID);
    void notifyRemoteSubscriber(Scope *sc, String &existingPrefixID, RemoteHost *subscriber, BABitvector *FIDToSubscriber);
    /**************************************************************************************************************************************/
    void notifyLocalPublisher(InformationItem *pub, BABitvector *FID);
    void notifyRemotePublisher(RemoteHost *remotePublisher, InformationItem *pub, BABitvector *FID, BABitvector *FIDToPublisher);
    /**************************************************************************************************************************************/
    void rendezvous(InformationItem *pub, RemoteHostSet &_subscribers);
    void requestTMAssistanceForRendezvous(InformationItem *pub, RemoteHostSet &_publishers, RemoteHostSet &_subscribers, IdsHashMap &IDs);
    void requestTMAssistanceForNotifyingSubscribers(Scope *sc, unsigned char request_type, RemoteHostSet &_subscribers, IdsHashMap &IDs);
    /***************members********************************/
    GlobalConf *gc;
    //TopologyManager *tm;
    RemoteHost *localProxy;
    /*scopes for which this node is the RV node*/
    ScopeHashMap scopeIndex;
    /*pubs for which this node is the RV node*/
    IIHashMap pubIndex;
    /*index containing potential publishers or subscribers*/
    /*the string here is a generic unique id - e.g. fd0 for a process, c4 for a click module, and the nodeID for a remote node*/
    RemoteHostHashMap pub_sub_Index;
    int number_of_neighbours;

    HashTable<String, Neighbour *> neighbours;
};

CLICK_ENDDECLS
#endif

