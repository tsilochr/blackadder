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

#ifndef CLICK_FORWARDER_HH
#define CLICK_FORWARDER_HH

#include "globalconf.hh"

#include <click/etheraddress.hh>
#include <clicknet/udp.h>

CLICK_DECLS

struct forwarding_entry {
    BABitvector *LID;
    EtherAddress *src;
    EtherAddress *dst;
    IPAddress *src_ip;
    IPAddress *dst_ip;
    int port;
};

class Forwarder : public Element {
public:
    Forwarder();
    ~Forwarder();
    const char *class_name() const {return "Forwarder";}
    const char *port_count() const {return "-/-";}
    const char *processing() const {return PUSH;}
    int configure(Vector<String>&, ErrorHandler*);
    int configure_phase() const{return 200;}
    int initialize(ErrorHandler *errh);
    void cleanup(CleanupStage stage);
    void push(int, Packet *);
    

    /*members*/
    GlobalConf *gc;
    atomic_uint32_t _id;
    int number_of_links;
    int proto_type;
    Vector<struct forwarding_entry *> forwarding_table;
};

CLICK_ENDDECLS
#endif


