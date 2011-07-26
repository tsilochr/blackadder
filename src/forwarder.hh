#ifndef CLICK_FORWARDER_HH
#define CLICK_FORWARDER_HH

#include "common.hh"

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

