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

#ifndef CLICK_TONETLINK_HH
#define CLICK_TONETLINK_HH

#include "netlink.hh"

CLICK_DECLS

class ToNetlink : public Element {
public:
    ToNetlink();
    ~ToNetlink();
    const char *class_name() const {return "ToNetlink";}
    const char *port_count() const {return "1/0";}//blackadder pushes packets to the only input
    const char *processing() const {return PUSH;}
    int configure(Vector<String>&, ErrorHandler*);
    int configure_phase() const {return 302;}
    int initialize(ErrorHandler *errh);
    void cleanup(CleanupStage stage);
    void push(int, Packet *);
#if CLICK_LINUXMODULE
    //bool run_task(Task *t);
#else
    void selected(int fd, int mask);
#endif
    /*members*/
    Netlink *netlink_element;
#if CLICK_LINUXMODULE
    //Task *_task;
#endif
    
};

CLICK_ENDDECLS
#endif
