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

#include "fromnetlink.hh"

CLICK_DECLS

#if CLICK_LINUXMODULE
static Vector<Packet *> *down_queue;
static Task *_st_task;
static unsigned long element_state;
static struct mutex down_mutex;
static PIDSet *_pid_set;
#else
static char *fake_buf;
#endif


#if CLICK_LINUXMODULE

void nl_callback(struct sk_buff *skb) {
    struct pid *_pid;
    unsigned long irq_flags;
    int type, flags, nlmsglen, skblen, pid;
    struct nlmsghdr *nlh;

    skblen = skb->len;
    if (skblen < sizeof (*nlh)) {
        return;
    }
    nlh = nlmsg_hdr(skb);
    nlmsglen = nlh->nlmsg_len;
    if (nlmsglen < sizeof (*nlh) || skblen < nlmsglen) {
        return;
    }
    flags = nlh->nlmsg_flags;
    type = nlh->nlmsg_type;
#if HAVE_SKB_DST_DROP
    skb_dst_drop(skb);
#else
    if (skb->dst) {
        dst_release(skb->dst);
        skb->dst = 0;
    }
#endif
    skb_orphan(skb);
    skb = skb_realloc_headroom(skb, 50);
    Packet *p = Packet::make(skb);

    /*pull the netlink header*/
    p->pull(sizeof (nlmsghdr));
    p->set_anno_u32(0, nlh->nlmsg_pid);

    mutex_lock(&down_mutex);
    down_queue->push_front(p);
    _st_task->reschedule();
    set_bit(TASK_IS_SCHEDULED, &element_state);
    /*get the struct *pid*/
    _pid = task_pid(current);
    /*get a reference to that task*/
    //put_pid seems not exported???!!! so i will not use get_pid either..May the god (of kernel) help me with that
    //get_pid(_pid);
    /*insert it to the hash set*/
    PIDSetItem pid_set_item = PIDSetItem(_pid);
    pid_set_item._pid_no = current->pid;
    /*this must be protected as well...the timer reads it*/
    _pid_set->find_insert(pid_set_item);
    mutex_unlock(&down_mutex);
    /*Done!*/
}
#endif

FromNetlink::FromNetlink() {
}

FromNetlink::~FromNetlink() {
}

int FromNetlink::configure(Vector<String> &conf, ErrorHandler *errh) {
    netlink_element = (Netlink *) cp_element(conf[0], this);
    //click_chatter("FromNetlink: configured!");
    return 0;
}

int FromNetlink::initialize(ErrorHandler *errh) {
#if CLICK_LINUXMODULE
    netlink_element->nl_sk = netlink_kernel_create(&init_net, NETLINK_BADDER, 0, nl_callback, NULL, THIS_MODULE);
    _task = new Task(this);
    _st_task = _task;
    _pid_set = new PIDSet();
    element_state = 0;
    down_queue = new Vector<Packet *>();
    mutex_init(&down_mutex);
    ScheduleInfo::initialize_task(this, _task, errh);
    _timer = new Timer(this);
    _timer->initialize(this);
    _timer->schedule_after_sec(1);
#else
    fake_buf = (char *) malloc(1);
    add_select(netlink_element->ss.fd, SELECT_READ);
#endif
    //click_chatter("FromNetlink: initialized!");
    return 0;
}

void FromNetlink::cleanup(CleanupStage stage) {
    if (stage >= CLEANUP_INITIALIZED) {
#if CLICK_LINUXMODULE
        int down_queue_size;
        /*delete the timer*/
        _timer->clear();
        delete _timer;
        _task->unschedule();
        delete _task;
        down_queue_size = down_queue->size();
        for (int i = 0; i < down_queue_size; i++) {
            //click_chatter("size of the packet at position %d is %d", (down_queue->size() - 1), (down_queue->at(down_queue->size() -1 ))->length());
            down_queue->at(down_queue->size() - 1)->kill();
            down_queue->pop_back();
        }
        /*delete queue*/
        delete down_queue;
        /*assume that all pids have been previously finished*/
        delete _pid_set;
#endif
    }
    //click_chatter("FromNetlink: Cleaned Up!");
}

#if CLICK_LINUXMODULE

void FromNetlink::run_timer(Timer *_timer) {
    WritablePacket *newPacket;
    unsigned char type;
    struct pid *_pid;
    struct task_struct *_ts;
    mutex_lock(&down_mutex);
    for (PIDSetIter it = _pid_set->begin(); it != _pid_set->end(); it++) {
        _pid = (*it)._pidpointer;
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 24)
        _ts = find_task_by_pid((*it)._pid_no);
#else
        _ts = get_pid_task(_pid, PIDTYPE_PID);
#endif
        if (_ts == NULL) {
            click_chatter("task %d is dead!!!", (*it)._pid_no);
            _pid_set->erase(it);
            //put_pid((*it)._pidpointer);
            newPacket = Packet::make(30, NULL, sizeof (type), 0);
            type = DISCONNECT;
            newPacket->set_anno_u32(0, (*it)._pid_no);
            memcpy(newPacket->data(), &type, sizeof (type));
            output(0).push(newPacket);
        }
    }
    mutex_unlock(&down_mutex);
    _timer->schedule_after_sec(5);
}
#endif

#if CLICK_LINUXMODULE

bool FromNetlink::run_task(Task *t) {
    int ret;
    Packet *down_packet = NULL;
    int pid;

    clear_bit(TASK_IS_SCHEDULED, &element_state);
    mutex_lock(&down_mutex);
    while (down_queue->size() > 0) {
        down_packet = down_queue->at(down_queue->size() - 1);
        down_queue->pop_back();
        output(0).push(down_packet);
    }
    if (test_bit(TASK_IS_SCHEDULED, &element_state) == 1) {
        t->fast_reschedule();
    }
    mutex_unlock(&down_mutex);
    return true;
}
#else

void FromNetlink::selected(int fd, int mask) {
    WritablePacket *newPacket;
    int total_buf_size;
    int bytes_read;
    if ((mask & SELECT_READ) == SELECT_READ) {
        /*read from the socket*/
        total_buf_size = recv(fd, fake_buf, 1, MSG_PEEK | MSG_TRUNC | MSG_WAITALL);
        if (total_buf_size < 0) {
            click_chatter("HMmmm");
            return;
        }
        newPacket = Packet::make(100, NULL, total_buf_size, 0);
        bytes_read = recv(fd, newPacket->data(), newPacket->length(), MSG_WAITALL);
        struct nlmsghdr *nlh = (struct nlmsghdr *) newPacket->data();
        /*pull the netlink header and message size, which is useless in kernel space (it's used in user space to allocate the appropriate buffer to receive the data)*/
        newPacket->pull(sizeof (nlmsghdr));
        newPacket->set_anno_u32(0, nlh->nlmsg_pid);
        /*wrap the sk buffer in a packet and add it to the down_queue*/
        output(0).push(newPacket);
    }
}
#endif

CLICK_ENDDECLS
EXPORT_ELEMENT(FromNetlink)
ELEMENT_REQUIRES(Netlink)
