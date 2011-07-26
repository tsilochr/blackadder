#ifndef CLICK_GLOBALCONF_HH
#define CLICK_GLOBALCONF_HH


#include "helper.hh"
#include "ba_bitvector.hh"
#include "common.hh"
CLICK_DECLS

class GlobalConf : public Element {
public:
    GlobalConf();
    ~GlobalConf();
    const char *class_name() const {return "GlobalConf";}
    const char *port_count() const {return "-/-";}
    const char *processing() const {return PUSH;}
    int configure(Vector<String>&, ErrorHandler*);
    int configure_phase() const{return 100;}
    int initialize(ErrorHandler *errh);
    void cleanup(CleanupStage stage);
   
    /****************************members***************************/
    /*the IP address where the IPC element binds to*/
    IPAddress ipaddr;
    /*the TCP port where the IPC element binds to*/
    int port;
    /*the node ID*/
    String nodeID;
    /*default LIPSIN identifier to the domain rendez-vous*/
    BABitvector *iLID;
    BABitvector *defaultRV_dl;
    /*only the RV node should know that FID*/
    BABitvector TMFID;
    /***************************************/
    bool use_mac;
    String RVScope;
    String TMScope;
    String nodeRVScope;
    String nodeTMScope;
    String notificationIID;
};

CLICK_ENDDECLS

#endif
