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

//#ifndef CLICK_IPCSERVER_HH
//#define CLICK_IPCSERVER_HH
//
//#include "helper.hh"
//#include "globalconf.hh"
//#include <arpa/inet.h>
//#include <unistd.h>
//#include <fcntl.h>
//
//#include <queue>
//
//CLICK_DECLS
//
//struct socket_state {
//    int fd;
//    bool reading_size;
//    /*these are for reading the buffer containing the size of the buffer coming*/
//    char *in_size_data;
//    int in_size_size;
//    int in_size_offset;
//    int in_size_remaining;
//    /***************************************************************************/
//    /*these are for reading the actual incoming data*/
//    WritablePacket *in_packet;
//    int in_data_size;
//    int in_data_offset;
//    int in_data_remaining;
//    /***************************************************************************/
//    std::queue <WritablePacket *> *out_buf_queue;
//};
//
//class IPCServer : public Element {
//public:
//    IPCServer();
//    ~IPCServer();
//
//    const char *class_name() const {return "IPCServer";}
//
//    const char *port_count() const {
//        return "1/1";
//    }
//
//    const char *processing() const {
//        return PUSH;
//    }
//    int configure(Vector<String>&, ErrorHandler*);
//
//    int configure_phase() const {
//        return 200;
//    }
//    int initialize(ErrorHandler *errh);
//    void cleanup(CleanupStage stage);
//    void push(int, Packet *);
//    void selected(int fd, int mask);
//    /*members*/
//    GlobalConf *gc;
//
//    HashTable <int, struct socket_state *> state_table;
//    int sockfd;
//    struct sockaddr_in serv_addr;
//};
//
//CLICK_ENDDECLS
//#endif
//
