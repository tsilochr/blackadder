#include "blackadder.hpp"
#include <stdio.h>

using namespace std;

int Blackadder::nextRegNum(){
 return getpid();
}

Blackadder::Blackadder(bool userspaceIn) {
    int ret;
    userspace = userspaceIn;
    disconnected = false;
    if (userspace) {
        cout << "Initializing blackadder client for user space" << endl;
        sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
	regNum = nextRegNum();
    } else {
        cout << "Initializing blackadder client for kernel space" << endl;
        sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_BADDER);
	regNum = getpid();
    }
    if (sock_fd < 0) {
        perror("socket");
    } else {
        cout << "Created and opened netlink socket" << endl;
    }
    /* source address */
    memset(&s_nladdr, 0, sizeof (s_nladdr));
    s_nladdr.nl_family = AF_NETLINK;
    s_nladdr.nl_pad = 0;
    s_nladdr.nl_pid = regNum;  

    ret = bind(sock_fd, (struct sockaddr*) &s_nladdr, sizeof (s_nladdr));
    if (ret < 0) {
        perror("bind");
    }
    /* destination address */
    memset(&d_nladdr, 0, sizeof (d_nladdr));
    d_nladdr.nl_family = AF_NETLINK;
    d_nladdr.nl_pad = 0;
    if (userspace) {
        d_nladdr.nl_pid = 9999; /* destined to user space blackadder */
    } else {
        d_nladdr.nl_pid = 0; /* destined to kernel */
    }
    fake_buf = (char *) malloc(1);
}

Blackadder::~Blackadder() {    
    free(fake_buf);
    if(userspace && !disconnected){
       disconnect(); 
    }
    if (sock_fd != -1) {
        cout << "Closed netlink socket" << endl;
        close(sock_fd);
    }
}

/*
 * One less muffer copy plz
 */
void Blackadder::create_and_send_buffers(unsigned char type, char *id_buff, int id_buff_len, char *prefix_id_buff, int prefix_id_buff_len, char strategy, char *LID) {
    struct msghdr msg;
    struct iovec *iov;
    unsigned char id_len = id_buff_len / PURSUIT_ID_LEN;
    unsigned char prefix_id_len = prefix_id_buff_len / PURSUIT_ID_LEN;
    struct nlmsghdr *nlh = (struct nlmsghdr *) malloc(sizeof (struct nlmsghdr));


    if (LID == NULL) {
        iov = (struct iovec *) calloc(sizeof (struct iovec), 7);
        /* Fill the netlink message header */
        nlh->nlmsg_len = sizeof (struct nlmsghdr) + 1 /*type*/ + 1 /*for id length*/ + id_buff_len + 1 /*for prefix_id length*/ + prefix_id_buff_len + 1 /*strategy*/;
        nlh->nlmsg_pid = regNum;
        nlh->nlmsg_flags = 1;
        nlh->nlmsg_type = 0;
    } else {
        iov = (struct iovec *) calloc(sizeof (struct iovec), 8);
        /* Fill the netlink message header */
        nlh->nlmsg_len = sizeof (struct nlmsghdr) + 1 /*type*/ + 1 /*for id length*/ + id_buff_len + 1 /*for prefix_id length*/ + prefix_id_buff_len + 1 /*strategy*/ + FID_LEN /*optional LID*/;
        nlh->nlmsg_pid = regNum;
        nlh->nlmsg_flags = 1;
        nlh->nlmsg_type = 0;
    }
    iov[0].iov_base = nlh;
    iov[0].iov_len = sizeof (struct nlmsghdr);
    iov[1].iov_base = &type;
    iov[1].iov_len = sizeof (type);
    iov[2].iov_base = &id_len;
    iov[2].iov_len = sizeof (id_len);
    iov[3].iov_base = (void *) id_buff;
    iov[3].iov_len = id_buff_len;
    iov[4].iov_base = &prefix_id_len;
    iov[4].iov_len = sizeof (prefix_id_len);
    iov[5].iov_base = (void *) prefix_id_buff;
    iov[5].iov_len = prefix_id_buff_len;
    iov[6].iov_base = &strategy;
    iov[6].iov_len = sizeof (strategy);
    if (LID == NULL) {
        memset(&msg, 0, sizeof (msg));
        msg.msg_name = (void *) &d_nladdr;
        msg.msg_namelen = sizeof (d_nladdr);
        msg.msg_iov = iov;
        msg.msg_iovlen = 7;
        sendmsg(sock_fd, &msg, 0);
    } else {

        iov[7].iov_base = (void *) LID;
        iov[7].iov_len = FID_LEN;


        memset(&msg, 0, sizeof (msg));
        msg.msg_name = (void *) &d_nladdr;
        msg.msg_namelen = sizeof (d_nladdr);
        msg.msg_iov = iov;
        msg.msg_iovlen = 8;
        sendmsg(sock_fd, &msg, 0);
    }
    free(iov);
    free(nlh);
}

void Blackadder::create_and_send_buffers(unsigned char type, string &id, string &prefix_id, char strategy, char *LID) {
    struct msghdr msg;
    struct iovec *iov;
    unsigned char id_len = id.length() / PURSUIT_ID_LEN;
    unsigned char prefix_id_len = prefix_id.length() / PURSUIT_ID_LEN;
    struct nlmsghdr *nlh = (struct nlmsghdr *) malloc(sizeof (struct nlmsghdr));


    if (LID == NULL) {
        iov = (struct iovec *) calloc(sizeof (struct iovec), 7);
        /* Fill the netlink message header */
        nlh->nlmsg_len = sizeof (struct nlmsghdr) + 1 /*type*/ + 1 /*for id length*/ + id.length() + 1 /*for prefix_id length*/ + prefix_id.length() + 1 /*strategy*/;
        nlh->nlmsg_pid = regNum;
        nlh->nlmsg_flags = 1;
        nlh->nlmsg_type = 0;
    } else {
        iov = (struct iovec *) calloc(sizeof (struct iovec), 8);
        /* Fill the netlink message header */
        nlh->nlmsg_len = sizeof (struct nlmsghdr) + 1 /*type*/ + 1 /*for id length*/ + id.length() + 1 /*for prefix_id length*/ + prefix_id.length() + 1 /*strategy*/ + FID_LEN /*optional LID*/;
        nlh->nlmsg_pid = regNum;
        nlh->nlmsg_flags = 1;
        nlh->nlmsg_type = 0;
    }
    iov[0].iov_base = nlh;
    iov[0].iov_len = sizeof (struct nlmsghdr);
    iov[1].iov_base = &type;
    iov[1].iov_len = sizeof (type);
    iov[2].iov_base = &id_len;
    iov[2].iov_len = sizeof (id_len);
    iov[3].iov_base = (void *) id.c_str();
    iov[3].iov_len = id.length();
    iov[4].iov_base = &prefix_id_len;
    iov[4].iov_len = sizeof (prefix_id_len);
    iov[5].iov_base = (void *) prefix_id.c_str();
    iov[5].iov_len = prefix_id.length();
    iov[6].iov_base = &strategy;
    iov[6].iov_len = sizeof (strategy);
    if (LID == NULL) {
        memset(&msg, 0, sizeof (msg));
        msg.msg_name = (void *) &d_nladdr;
        msg.msg_namelen = sizeof (d_nladdr);
        msg.msg_iov = iov;
        msg.msg_iovlen = 7;
        sendmsg(sock_fd, &msg, 0);
    } else {

        iov[7].iov_base = (void *) LID;
        iov[7].iov_len = FID_LEN;


        memset(&msg, 0, sizeof (msg));
        msg.msg_name = (void *) &d_nladdr;
        msg.msg_namelen = sizeof (d_nladdr);
        msg.msg_iov = iov;
        msg.msg_iovlen = 8;
        sendmsg(sock_fd, &msg, 0);
    }
    free(iov);
    free(nlh);
}

void Blackadder::disconnect() {
    disconnected = true;
    cout << "disconnecting\n";
    struct msghdr msg;
    struct iovec *iov;
    struct nlmsghdr *nlh = (struct nlmsghdr *) malloc(sizeof (struct nlmsghdr));
    unsigned char type = DISCONNECT;
    /* Fill the netlink message header */
    nlh->nlmsg_len = sizeof (struct nlmsghdr) + 1 /*type*/;
    nlh->nlmsg_pid = regNum;
    nlh->nlmsg_flags = 1;
    nlh->nlmsg_type = 0;
    iov = new iovec[2];
    iov[0].iov_base = nlh;
    iov[0].iov_len = sizeof (struct nlmsghdr);
    iov[1].iov_base = &type;
    iov[1].iov_len = sizeof (type);
    memset(&msg, 0, sizeof (msg));
    msg.msg_name = (void *) &d_nladdr;
    msg.msg_namelen = sizeof (d_nladdr);
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;
    sendmsg(sock_fd, &msg, 0);
    free(nlh);
    delete [] iov;
    close(sock_fd);
    cout << "Closed netlink socket" << endl;
    sock_fd = -1;
}

void Blackadder::publish_scope(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID){
    create_and_send_buffers(PUBLISH_SCOPE, id, id_len, prefix_id, prefix_len, strategy, LID);
}

void Blackadder::publish_info(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID) {
    create_and_send_buffers(PUBLISH_INFO, id, id_len, prefix_id, prefix_len, strategy, LID);
}

void Blackadder::unpublish_scope(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID) {
    create_and_send_buffers(UNPUBLISH_SCOPE, id, id_len, prefix_id, prefix_len, strategy, LID);
}

void Blackadder::unpublish_info(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID) {
    create_and_send_buffers(UNPUBLISH_INFO, id, id_len, prefix_id, prefix_len, strategy, LID);
}

void Blackadder::subscribe_scope(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID) {
    create_and_send_buffers(SUBSCRIBE_SCOPE, id, id_len, prefix_id, prefix_len, strategy, LID);
}

void Blackadder::subscribe_info(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID) {
    create_and_send_buffers(SUBSCRIBE_INFO, id, id_len, prefix_id, prefix_len, strategy, LID);
}

void Blackadder::unsubscribe_scope(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID) {
    create_and_send_buffers(UNSUBSCRIBE_SCOPE, id, id_len, prefix_id, prefix_len, strategy, LID);
}

void Blackadder::unsubscribe_info(char *id, int id_len, char *prefix_id, int prefix_len, char strategy, char *LID) {
    create_and_send_buffers(UNSUBSCRIBE_INFO, id, id_len, prefix_id, prefix_len, strategy, LID);
}

//void Blackadder::publish_data(string &id, char strategy, char * LID, char *data, int data_len)
void Blackadder::publish_data(char *id, int id_buff_len, char strategy, char * LID, char *data, int data_len) {
    struct msghdr msg;
    struct iovec *iov;
    unsigned char type = PUBLISH_DATA;
    unsigned char id_len = id_buff_len / PURSUIT_ID_LEN;
    struct nlmsghdr *nlh = (struct nlmsghdr *) malloc(sizeof (struct nlmsghdr));
    if (LID == NULL) {
        /* Fill the netlink message header */
        nlh->nlmsg_len = sizeof (struct nlmsghdr) + 1 /*type*/ + 1 /*for id length*/ + id_buff_len + sizeof (strategy) + data_len;
        nlh->nlmsg_pid = regNum;
        nlh->nlmsg_flags = 1;
        nlh->nlmsg_type = 0;
        iov = (struct iovec *) calloc(sizeof (struct iovec), 6);
    } else {
        /* Fill the netlink message header */
        nlh->nlmsg_len = sizeof (struct nlmsghdr) + 1 /*type*/ + 1 /*for id length*/ + id_buff_len + sizeof (strategy) + FID_LEN + data_len;
        nlh->nlmsg_pid = regNum;
        nlh->nlmsg_flags = 1;
        nlh->nlmsg_type = 0;
        iov = (struct iovec *) calloc(sizeof (struct iovec), 7);
    }
    iov[0].iov_base = nlh;
    iov[0].iov_len = sizeof (struct nlmsghdr);
    iov[1].iov_base = &type;
    iov[1].iov_len = sizeof (type);
    iov[2].iov_base = &id_len;
    iov[2].iov_len = sizeof (id_len);
    iov[3].iov_base = (void *) id;
    iov[3].iov_len = id_buff_len;
    iov[4].iov_base = (void *) &strategy;
    iov[4].iov_len = sizeof (unsigned char);
    if (LID == NULL) {
        iov[5].iov_base = (void *) data;
        iov[5].iov_len = data_len;
        memset(&msg, 0, sizeof (msg));
        msg.msg_name = (void *) &d_nladdr;
        msg.msg_namelen = sizeof (d_nladdr);
        msg.msg_iov = iov;
        msg.msg_iovlen = 6;
        sendmsg(sock_fd, &msg, MSG_WAITALL);
    } else {
        iov[5].iov_base = (void *) LID;
        iov[5].iov_len = FID_LEN;
        iov[6].iov_base = (void *) data;
        iov[6].iov_len = data_len;
        memset(&msg, 0, sizeof (msg));
        msg.msg_name = (void *) &d_nladdr;
        msg.msg_namelen = sizeof (d_nladdr);
        msg.msg_iov = iov;
        msg.msg_iovlen = 7;
        sendmsg(sock_fd, &msg, MSG_WAITALL);
    }
    free(nlh);
    free(iov);
}

Event *Blackadder::getNewEvent(){
    int total_buf_size = 0;
    int bytes_read;
    int data_len;
    unsigned char type;
    unsigned char id_len;
    char *data;
    char *buf;
    total_buf_size = recv(sock_fd, fake_buf, 1, MSG_PEEK | MSG_TRUNC | MSG_WAITALL);
    if (total_buf_size > 0) {
        buf = (char *) malloc(total_buf_size);

        bytes_read = recv(sock_fd, buf, total_buf_size, MSG_WAITALL);
        //cout << "bytes_read: " << bytes_read << endl;
        type = *(buf + sizeof (struct nlmsghdr));
        id_len = *(buf + sizeof (struct nlmsghdr) + sizeof (unsigned char));
        string str_id(buf + sizeof (struct nlmsghdr) + sizeof (unsigned char) + sizeof (unsigned char), ((int) id_len) * PURSUIT_ID_LEN);
        if (type == PUBLISHED_DATA) {
            data = buf + sizeof (struct nlmsghdr) + sizeof (unsigned char) + sizeof (unsigned char) + ((int) id_len) * PURSUIT_ID_LEN;
            data_len = total_buf_size - (sizeof (struct nlmsghdr) + sizeof (unsigned char) + sizeof (unsigned char) + ((int) id_len) * PURSUIT_ID_LEN);
        } else {
            data = NULL;
            data_len = 0;
        }
        return new Event(type, str_id, data, data_len, buf);
    } else {
        return new Event(0, "", NULL, 0, NULL);
    }
}

Event::Event(unsigned char _type, string _id, char *_data, int _data_len, char *_buffer) {
    type = _type;
    id = _id;
    data_len = _data_len;
    data = _data;
    buffer = _buffer;
}
