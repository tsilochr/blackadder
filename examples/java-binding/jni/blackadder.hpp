#ifndef BLACKADDER_HPP
#define BLACKADDER_HPP

#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <vector>
#include <list>
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
    void publish_scope(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID);
    void publish_info(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID);

    void unpublish_scope(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID);
    void unpublish_info(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID);
    void subscribe_scope(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID);
    void subscribe_info(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID);
    void unsubscribe_scope(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID);
    void unsubscribe_info(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID);
    void publish_data(char *id, int id_len, char strategy, char * LID, char *data, int data_len);

    Event *getNewEvent();
    char *fake_buf;
//private:
    void create_and_send_buffers(unsigned char type, string &id, string &prefix_id, char strategy, char *LID);
    void create_and_send_buffers(unsigned char type, char *id_buff, int id_buff_len, char *prefix_id_buff, int prefix_id_buff_len, char strategy, char *LID);
    int nextRegNum();
    /*members*/
    int sock_fd;
    struct sockaddr_nl s_nladdr, d_nladdr;
    int regNum;
    bool userspace;
    bool disconnected;
    static int counter;
    static list<int> pool;
};

class Event {
public:
    Event(unsigned char _type, string _id, char *_data, int _data_len, char *_buffer);    
    unsigned char type;
    string id;
    char *data;
    int data_len;
    char *buffer;
};

#endif
