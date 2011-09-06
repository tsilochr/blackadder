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

#ifndef CLICK_FROMNETLINK_HH
#define CLICK_FROMNETLINK_HH

#include "netlink.hh"
#include "helper.hh"

CLICK_DECLS

class FromNetlink : public Element {
public:
    FromNetlink();
    ~FromNetlink();
    const char *class_name() const {return "FromNetlink";}
    const char *port_count() const {return "0/1";}//pushes packets to blackadder via the only output
    const char *processing() const {return PUSH;}
    int configure(Vector<String>&, ErrorHandler*);
    int configure_phase() const {return 301;}
    int initialize(ErrorHandler *errh);
    void cleanup(CleanupStage stage);
#if CLICK_LINUXMODULE
    bool run_task(Task *t);
    void run_timer(Timer *_timer);
#else
    void selected(int fd, int mask);
#endif
    /*members*/
    Netlink *netlink_element;
#if CLICK_LINUXMODULE
    Task *_task;
    Timer *_timer;
#endif
};

CLICK_ENDDECLS
#endif
