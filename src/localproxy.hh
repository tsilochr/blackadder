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

#ifndef CLICK_LOCALPROXY_HH
#define CLICK_LOCALPROXY_HH

#include "globalconf.hh"
#include "localhost.hh"
#include "activepub.hh"

CLICK_DECLS

class LocalHost;

class LocalProxy : public Element {
public:
    LocalProxy();
    ~LocalProxy();

    const char *class_name() const {return "LocalProxy";}

    const char *port_count() const {
        return "-/-";
    }

    const char *processing() const {
        return PUSH;
    }
    int configure(Vector<String>&, ErrorHandler*);

    int configure_phase() const {
        return 200;
    }
    int initialize(ErrorHandler *errh);
    void cleanup(CleanupStage stage);
    void push(int, Packet *);

    bool handleLocalRequest(unsigned char &type, LocalHost *_localhost, String &ID, String &prefixID, unsigned char &strategy, BABitvector &RVFID);
    void handleUserPublication(String &ID, Packet *p, LocalHost *_localhost);
    void handleUserPublication(String &ID, BABitvector &FID_to_subscribers, Packet *p, LocalHost *_localhost);
    void handleNetworkPublication(Vector<String> &IDs, Packet *p);
    void pushDataToLocalSubscriber(LocalHost *_localhost, String &ID, Packet *p);

    void pushDataToRemoteSubscribers(ActivePublication *aiip, Packet *p);
    void pushDataToRemoteSubscribers(Vector<String> &IDs, BABitvector &FID_to_subscribers, Packet *p);
    
    void sendReqToRV(Packet *p, BABitvector &FID);
    void createAndSendPacketToRV(unsigned char type, unsigned char IDLength, String &ID, unsigned char prefixIDLength, String &prefixID, BABitvector &RVFID, unsigned char strategy);
    void processRVNotification(Packet *p);

    bool storeActivePublication(LocalHost *_localhost, String &fullID, unsigned char strategy, BABitvector & RVFID, bool isScope);
    bool storeActiveSubscription(LocalHost *_localhost, String &fullID, unsigned char strategy, BABitvector & RVFID, bool isScope);
    bool removeActivePublication(LocalHost *_localhost, String &fullID, unsigned char strategy);
    bool removeActiveSubscription(LocalHost *_localhost, String &fullID, unsigned char strategy);

    void connect(int type, int id);
    void disconnect(int type, int id);

    LocalHost * createLocalHost(int type, int id);
    LocalHost * checkLocalHost(int type, int id);

    void deleteAllActiveInformationItemPublications(LocalHost * _localhost);
    void deleteAllActiveInformationItemSubscriptions(LocalHost * _localhost);
    void deleteAllActiveScopePublications(LocalHost * _localhost);
    void deleteAllActiveScopeSubscriptions(LocalHost * _localhost);

    bool findLocalSubscribers(Vector<String> &IDs, LocalHostStringHashMap & _localSubscribers);
    void findActiveSubscriptions(String &ID, LocalHostSet &local_subscribers_to_notify);
    void sendNotificationLocally(unsigned char type, LocalHost *_localhost, String ID);

    /****************members*******************/
    GlobalConf *gc;

    PubSubIdx local_pub_sub_Index;
    ActivePub activePublicationIndex;
    ActiveSub activeSubscriptionIndex;
};



CLICK_ENDDECLS
#endif
