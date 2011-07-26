#include "netlink.hh"

CLICK_DECLS

Netlink::Netlink() {
}

Netlink::~Netlink() {
}

//int Netlink::configure(Vector<String> &conf, ErrorHandler *errh) {
//    //click_chatter("Netlink: configured!");
//    return 0;
//}

int Netlink::initialize(ErrorHandler *errh) {
#if CLICK_LINUXMODULE

#else
    int ret;
    struct sockaddr_nl s_nladdr;
    ss.fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    ss.out_buf_queue = new std::queue<struct WritablePacket *>();
    /* source address */
    memset(&s_nladdr, 0, sizeof (s_nladdr));
    s_nladdr.nl_family = AF_NETLINK;
    s_nladdr.nl_pad = 0;
    s_nladdr.nl_pid = 9999;
    ret = bind(ss.fd, (struct sockaddr*) &s_nladdr, sizeof (s_nladdr));
    if (ret == -1) {
        perror("bind: ");
        return -1;
    }
#endif
    //click_chatter("Netlink: initialized!");
    return 0;
}

void Netlink::cleanup(CleanupStage stage) {
    if (stage >= CLEANUP_INITIALIZED) {
#if CLICK_LINUXMODULE
        /*release the socket*/
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 24)
        sock_release(nl_sk->sk_socket);
#else
        netlink_kernel_release(nl_sk);
#endif
#else
        delete ss.out_buf_queue;
        close(ss.fd);
#endif
    }
    //click_chatter("Netlink: Cleaned up!");
}

CLICK_ENDDECLS
EXPORT_ELEMENT(Netlink)
