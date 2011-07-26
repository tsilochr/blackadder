//#include "ipcserver.hh"
//
//CLICK_DECLS
//IPCServer::IPCServer() {
//}
//
//IPCServer::~IPCServer() {
//}
//
//int IPCServer::configure(Vector<String> &conf, ErrorHandler *errh) {
//    gc = (GlobalConf *) cp_element(conf[0], this);
//    return 0;
//}
//
//int IPCServer::initialize(ErrorHandler *errh) {
//    int optval;
//    sockfd = socket(AF_INET, SOCK_STREAM, 0);
//    // set SO_REUSEADDR
//    optval = 1;
//    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));
//    //setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &optval, sizeof (optval));
//    fcntl(sockfd, F_SETFL, O_NONBLOCK);
//    if (sockfd < 0) {
//        click_chatter("IPCSERVER: Could not create socket\n");
//    }
//    bzero((char *) &serv_addr, sizeof (serv_addr));
//    serv_addr.sin_family = AF_INET;
//    inet_aton(gc->ipaddr.unparse().c_str(), &serv_addr.sin_addr);
//    serv_addr.sin_port = htons(gc->port);
//    click_chatter("IPCServer: binding to address %s, port %d\n", inet_ntoa(serv_addr.sin_addr), gc->port);
//
//    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
//        click_chatter("IPCSERVER: ERROR on binding\n");
//    }
//    listen(sockfd, 5);
//    add_select(sockfd, SELECT_READ);
//    return 0;
//}
//
//void IPCServer::cleanup(CleanupStage stage) {
//    /*I should close all open sockets*/
//    for (HashTable <int, struct socket_state *>::iterator it = state_table.begin(); it != state_table.end(); it++) {
//        struct socket_state *ss = (struct socket_state *) it.value();
//        click_chatter("closing fd %d", ss->fd);
//        /*I have to free some more things here but I don't care right now*/
//        /*TODO:*/
//        close(ss->fd);
//        free(ss);
//    }
//    close(sockfd);
//    //click_chatter("IPCServer: Cleanup myself!!!");
//}
//
//void IPCServer::push(int, Packet *p) {
//    WritablePacket *wp;
//    int htonl_size;
//    int fd;
//    struct socket_state *ss;
//    memcpy(&fd, p->data(), sizeof (int));
//    ss = state_table.get(fd);
//    if (ss != state_table.default_value()) {
//        //click_chatter("IPCSERVER: sending to fd %d a packet of length %d", fd, p->length());
//        wp = p->uniqueify();
//        htonl_size = htonl(p->length() - sizeof (int));
//        memcpy(wp->data(), &htonl_size, sizeof (int));
//        /*Here I have to find the socket (ss)to enqueue the buffer (the fd is in the packet)*/
//        ss = state_table.get(fd);
//        ss->out_buf_queue->push(wp);
//        add_select(ss->fd, SELECT_WRITE);
//        //remove_select(ss->fd, SELECT_READ);
//        //click_chatter("IPCSERVER: PUSHED DATA TO THE QUEUE");
//    }
//}
//
//void IPCServer::selected(int fd, int mask) {
//    int newfd;
//    int bytes;
//    unsigned char type;
//    WritablePacket *newPacket;
//    struct socket_state *ss;
//    if ((mask & SELECT_READ) == SELECT_READ) {
//        if (fd == sockfd) {
//            /*this is a connection request to the server socket*/
//            int len = sizeof (serv_addr);
//            newfd = accept(fd, (struct sockaddr *) &serv_addr, (socklen_t *) & len);
//            if (newfd < 0) {
//                click_chatter("IPCSERVER: ERROR on accept");
//            } else {
//                click_chatter("IPCSERVER: Accepted Connection - FD: %d", fd);
//                click_chatter("IPCSERVER: Accepted Connection - newFD: %d", newfd);
//                int optval = 1;
//                setsockopt(newfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));
//                //setsockopt(newfd, IPPROTO_TCP, TCP_NODELAY, (char *) &optval, sizeof (optval));
//                fcntl(newfd, F_SETFL, O_NONBLOCK);
//                /*allocate buffers and register socket for reading*/
//                struct socket_state *ss = (struct socket_state *) malloc(sizeof (struct socket_state));
//                ss->fd = newfd;
//                ss->out_buf_queue = new std::queue<struct WritablePacket *>();
//                ss->in_size_size = 4;
//                ss->in_size_remaining = ss->in_size_size;
//                ss->in_size_offset = 0;
//                ss->in_size_data = (char *) malloc(ss->in_size_size);
//                state_table.set(newfd, ss);
//                add_select(newfd, SELECT_READ);
//                remove_select(newfd, SELECT_WRITE);
//                ss->reading_size = true;
//                /*push a connection notification to the local proxy element*/
//                newPacket = Packet::make(0, NULL, sizeof (newfd) + sizeof (type), 0);
//                type = CONNECT;
//                memcpy(newPacket->data(), &newfd, sizeof (newfd));
//                memcpy(newPacket->data() + sizeof (newfd), &type, sizeof (type));
//                output(0).push(newPacket);
//            }
//        } else {
//            ss = state_table.get(fd);
//            if (ss != state_table.default_value()) {
//                /*this fd is a slave socket...have to find the respective struct socket_state*/
//                /*read from the socket*/
//                if (ss->reading_size) {
//                    //click_chatter("1)/*socket is readable*/");
//                    /*here I have to read the size of the incoming buffer*/
//                    //click_chatter("IPCSERVER: reading from FD: %d", fd);
//                    //click_chatter("IPCSERVER: **I will try to read %d bytes", ss->in_size_remaining);
//                    bytes = read(fd, ss->in_size_data + ss->in_size_offset, ss->in_size_remaining);
//                    if (bytes == 0) {
//                        /*socket is probably disconnected...free all state*/
//                        free(ss->in_size_data);
//                        state_table.erase(fd);
//                        /*maybe there are some packets stuck in the out queue - free them all*/
//                        while (!ss->out_buf_queue->empty()) {
//                            ss->out_buf_queue->front()->kill();
//                            ss->out_buf_queue->pop();
//                        }
//                        delete(ss->out_buf_queue);
//                        free(ss);
//                        remove_select(fd, SELECT_READ);
//                        remove_select(fd, SELECT_WRITE);
//                        close(fd);
//                        newPacket = Packet::make(0, NULL, sizeof (type) + sizeof (fd), 0);
//                        type = DISCONNECT;
//                        click_chatter("Socket %d disconnected", fd);
//                        memcpy(newPacket->data(), &fd, sizeof (fd));
//                        memcpy(newPacket->data() + sizeof (fd), &type, sizeof (type));
//                        output(0).push(newPacket);
//                        return;
//                    }
//                    //click_chatter("IPCSERVER: **I read %d bytes", bytes);
//                    ss->in_size_offset = ss->in_size_offset + bytes;
//                    ss->in_size_remaining = ss->in_size_remaining - bytes;
//                    //click_chatter("IPCSERVER: offset: %d\n", ss->in_size_offset);
//                    //click_chatter("IPCSERVER: remaining: %d\n", ss->in_size_remaining);
//                    if (ss->in_size_remaining == 0) {
//                        //click_chatter("IPCSERVER: read size!!!!!\n");
//                        int size = 0;
//                        /*allocate the buffer for the incoming data...*/
//                        memcpy(&size, ss->in_size_data, ss->in_size_size);
//                        //click_chatter("IPCSERVER: I have to read a buffer of %d bytes!",ntohl(size));
//                        /*reset the size buffer - do not free it...will be reused*/
//                        ss->in_size_size = 4;
//                        ss->in_size_remaining = ss->in_size_size;
//                        ss->in_size_offset = 0;
//                        /*initialize the data buffer*/
//                        ss->in_packet = Packet::make(100, NULL,ntohl(size), 0);
//                        ss->in_data_size = ss->in_packet->length();
//                        ss->in_data_remaining = ss->in_data_size;
//                        ss->in_data_offset = 0;
//                        ss->reading_size = false;
//                    }
//                } else {
//                    //click_chatter("2)/*socket is readable*/");
//                    /*here I have to read the actual data of the incoming buffer*/
//                    //click_chatter("IPCSERVER: reading from FD: %d", fd);
//                    //click_chatter("IPCSERVER: I will try to read %d bytes", ss->in_data_remaining);
//                    bytes = read(fd, ss->in_packet->data() + ss->in_data_offset, ss->in_data_remaining);
//                    if (bytes == 0) {
//                        /*I was reading data so I had allocated a packet - kill it*/
//                        ss->in_packet->kill();
//                        /*socket is probably disconnected...free all state*/
//                        free(ss->in_size_data);
//                        state_table.erase(fd);
//                        /*maybe there are some packets stuck in the out queue - free them all*/
//                        while (!ss->out_buf_queue->empty()) {
//                            ss->out_buf_queue->front()->kill();
//                            ss->out_buf_queue->pop();
//                        }
//                        delete(ss->out_buf_queue);
//                        free(ss);
//                        remove_select(fd, SELECT_READ);
//                        remove_select(fd, SELECT_WRITE);
//                        close(fd);
//                        newPacket = Packet::make(0, NULL, sizeof (type) + sizeof (fd), 0);
//                        type = DISCONNECT;
//                        click_chatter("Socket %d disconnected", fd);
//                        memcpy(newPacket->data(), &fd, sizeof (fd));
//                        memcpy(newPacket->data() + sizeof (fd), &type, sizeof (type));
//                        output(0).push(newPacket);
//                        return;
//                    }
//                    //click_chatter("IPCSERVER: I read %d bytes", bytes);
//                    ss->in_data_offset = ss->in_data_offset + bytes;
//                    ss->in_data_remaining = ss->in_data_remaining - bytes;
//                    //click_chatter("IPCSERVER: offset: %d\n", ss->in_data_offset);
//                    //click_chatter("IPCSERVER: remaining: %d\n", ss->in_data_remaining);
//                    if (ss->in_data_remaining == 0) {
//                        //click_chatter("IPCSERVER: read data\n");
//                        ss->reading_size = true;
//                        WritablePacket *wp = ss->in_packet->push(sizeof(int));
//                        memcpy(wp->data(), &fd, sizeof (int));
//                        output(0).push(wp);
//                    }
//                }
//            }
//        }
//    }
//    if ((mask & SELECT_WRITE) == SELECT_WRITE) {
//        //click_chatter("2)/*socket is writable*/");
//        /*socket is writable*/
//        ss = state_table.get(fd);
//        if (ss != state_table.default_value()) {
//            if (!ss->out_buf_queue->empty()) {
//                newPacket = ss->out_buf_queue->front();
//                bytes = write(fd, newPacket->data(), newPacket->length());
//                //click_chatter("IPCSERVER: I wrote %d bytes", bytes);
//                newPacket->pull(bytes);
//                if (newPacket->length() == 0) {
//                    /*remove buffer from queue and free it*/
//                    newPacket->kill();
//                    ss->out_buf_queue->pop();
//                    if (ss->out_buf_queue->empty()) {
//                        //click_chatter("IPCSERVER: QUEUE IS EMPTY NOW.....");
//                        remove_select(fd, SELECT_WRITE);
//                    }
//                }
//            } else {
//                //click_chatter("oops...queue is empty");
//                remove_select(fd, SELECT_WRITE);
//                add_select(fd, SELECT_READ);
//            }
//        }
//    }
//}
//
//CLICK_ENDDECLS
//EXPORT_ELEMENT(IPCServer)
//
