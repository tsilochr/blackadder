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

#ifndef BLACKADDER_HPP
#define BLACKADDER_HPP

#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <vector>
#include <sstream> 
#include <iostream>

#include "bitvector.hpp"

#define PUBLISH_SCOPE 0
#define PUBLISH_INFO 1
#define UNPUBLISH_SCOPE 2
#define UNPUBLISH_INFO 3
#define SUBSCRIBE_SCOPE 4
#define SUBSCRIBE_INFO 5
#define UNSUBSCRIBE_SCOPE 6
#define UNSUBSCRIBE_INFO 7
#define PUBLISH_DATA  8 //the request
#define DISCONNECT 13

#define PURSUIT_ID_LEN 8 //in bytes
#define FID_LEN 8 //in bytes
#define NODEID_LEN PURSUIT_ID_LEN //in bytes

#define NODE_LOCAL 0
#define LINK_LOCAL 1
#define DOMAIN_LOCAL 2
#define PUBLISH_NOW 3
#define SUBSCRIBE_LOCALLY 4

#define START_PUBLISH 100
#define STOP_PUBLISH 101
#define SCOPE_PUBLISHED 102
#define SCOPE_UNPUBLISHED 103
#define PUBLISHED_DATA 104

#define NETLINK_BADDER 20
class Event;

class Blackadder {
public:
    Blackadder(bool user_space);
    ~Blackadder();
    void disconnect();
    void publish_scope(string &id, string &prefix_id, char strategy, char *LID);
    void publish_info(string &id, string &prefix_id, char strategy, char *LID);
    void unpublish_scope(string &id, string &prefix_id, char strategy, char *LID);
    void unpublish_info(string &id, string &prefix_id, char strategy, char *LID);
    void subscribe_scope(string &id, string &prefix_id, char strategy, char *LID);
    void subscribe_info(string &id, string &prefix_id, char strategy, char *LID);
    void unsubscribe_scope(string &id, string &prefix_id, char strategy, char *LID);
    void unsubscribe_info(string &id, string &prefix_id, char strategy, char *LID);
    void publish_data(string &id, char strategy, char * LID, char *data, int data_len);
    Event getEvent();
    char *fake_buf;
//private:
    void create_and_send_buffers(unsigned char type, string &id, string &prefix_id, char strategy, char *LID);
    /*members*/
    int sock_fd;
    struct sockaddr_nl s_nladdr, d_nladdr;
};

class Event {
public:
    Event(unsigned char _type, string _id, char *_data, int _data_len, char *_buffer);
    ~Event();
    unsigned char type;
    string id; /*in Hex*/
    char *data;
    int data_len;
    char *buffer; /*do not use that...only the destructor uses it to delete the whole buffer once*/
};

#endif