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

#ifndef CLICK_NETLINK_HH
#define CLICK_NETLINK_HH

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/element.hh>

#include <click/string.hh>
#include <click/hashtable.hh>
#include <click/vector.hh>
#include <click/bitvector.hh>
#include <click/straccum.hh>

#include <click/task.hh>
#include <click/standard/scheduleinfo.hh>
#include <click/timer.hh>
#include "common.hh"

#if CLICK_LINUXMODULE
#include <click/cxxprotect.h>

CLICK_CXX_PROTECT
#include <linux/pid.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/net.h>
CLICK_CXX_UNPROTECT
#include <click/cxxunprotect.h>
#define TASK_IS_SCHEDULED 0
#else
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <arpa/inet.h>
#include <queue>
#include <click/cxxprotect.h>
CLICK_CXX_PROTECT
#include <linux/netlink.h>
CLICK_CXX_UNPROTECT
#include <click/cxxunprotect.h>

struct socket_state {
    int fd;
    WritablePacket *in_packet;
    std::queue <WritablePacket *> *out_buf_queue;
};
#endif

CLICK_DECLS

class Netlink : public Element {
public:
    Netlink();
    ~Netlink();
    const char *class_name() const {return "Netlink";}
    const char *port_count() const {return "0/0";}
    const char *processing() const {return AGNOSTIC;} //actually it does not process packets. It is the base element for ToNetlink and FromNetlink.}
    //int configure(Vector<String>&, ErrorHandler*);
    /*It should be initialized BEFORE the FromNetlink and ToNetlink*/
    int configure_phase() const {return 300;}
    int initialize(ErrorHandler *errh);
    void cleanup(CleanupStage stage);
    /*members*/
#if CLICK_LINUXMODULE
    struct sock *nl_sk;
#else
    struct socket_state ss;
#endif
};

CLICK_ENDDECLS
#endif
