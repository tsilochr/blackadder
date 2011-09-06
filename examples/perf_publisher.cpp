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

#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <string.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <sstream> 
#include <string> 
#include <vector> 
#include <sys/time.h>

#include "bitvector.hpp"
#include "blackadder.hpp"

Blackadder *ba;
int counter = 0;
struct timezone tz;

struct timeval start_tv;
struct timeval end_tv;
struct timeval duration;

int payload_size = 1200;
char *payload = (char *) malloc(payload_size);
char *end_payload = (char *) malloc(payload_size);

bool experiment_started = false;

using namespace std;

string hex_to_chararray(string const& hexstr) {
    vector<unsigned char> bytes = vector<unsigned char>();
    for (string::size_type i = 0; i < hexstr.size() / 2; ++i) {
        istringstream iss(hexstr.substr(i * 2, 2));
        unsigned int n;
        iss >> hex >> n;
        bytes.push_back(static_cast<unsigned char> (n));
    }
    bytes.data();
    string result = string((const char *) bytes.data(), bytes.size());
    return result;
}

string chararray_to_hex(const string &str) {
    ostringstream oss;
    for (string::size_type i = 0; i < str.size(); ++i) {
        if ((unsigned short) (unsigned char) str[i] > 15) {
            oss << hex << (unsigned short) (unsigned char) str[i];
        } else {
            oss << hex << '0';
            oss << hex << (unsigned short) (unsigned char) str[i];
        }
    }
    return oss.str();
}

void *event_listener_loop(void *arg) {
    Blackadder *ba = (Blackadder *) arg;
    Event ev = ba->getEvent();
    if (ev.type == START_PUBLISH) {
        for (int i = 0; i < 1000000; i++) {
            //cout << "publishing data for ID " << chararray_to_hex(ev.id) << endl;
            
            ba->publish_data(ev.id, NODE_LOCAL, NULL, payload, payload_size);
            //ba->publish_data(ev.id, DOMAIN_LOCAL, NULL, payload, payload_size);
            //ba->publish_data(ev.id, LINK_LOCAL, NULL, payload, payload_size);
        }
        for (int i = 0; i < 100; i++) {
            //cout << "publishing end flag for ID " << chararray_to_hex(ev.id) << endl;
            ba->publish_data(ev.id, NODE_LOCAL, NULL, end_payload, payload_size);
            //ba->publish_data(ev.id, DOMAIN_LOCAL, NULL, end_payload, payload_size);
            //ba->publish_data(ev.id, LINK_LOCAL, NULL, end_payload, payload_size);
        }
    }

    return 0;
}

void sigfun(int sig) {
    (void) signal(SIGINT, SIG_DFL);
    ba->disconnect();
    free(payload);
    free(end_payload);
    exit(0);
}

int main(int argc, char* argv[]) {
    pthread_t event_listener;

    //string link_local_LID("0000000000000000000000000000000000000000000000000000000000000010");
    //Bitvector bin_LID = Bitvector(link_local_LID);
    
    memset(payload, 'A', payload_size);
    memset(end_payload, 'B', payload_size);
    (void) signal(SIGINT, sigfun);

    if (argc > 1) {
        int user_or_kernel = atoi(argv[1]);
        if (user_or_kernel == 0) {
            ba = new Blackadder(true);
        } else {
            ba = new Blackadder(false);
        }
    } else {
        /*By Default I assume blackadder is running in user space*/
        ba = new Blackadder(true);
    }
    cout << "Process ID: " << getpid() << endl;

    pthread_create(&event_listener, NULL, event_listener_loop, (void *) ba);

    string id = "1111111111111111";
    string prefix_id = string();
    string bin_id = hex_to_chararray(id);
    string bin_prefix_id = hex_to_chararray(prefix_id);
    
    ba->publish_scope(bin_id, prefix_id, NODE_LOCAL, NULL);
    //ba->publish_scope(bin_id, prefix_id, DOMAIN_LOCAL, NULL);
    //ba->publish_scope(bin_id, prefix_id, LINK_LOCAL, (char *) bin_LID._data);
    
    id = "1111111111111111";
    prefix_id = "1111111111111111";
    bin_id = hex_to_chararray(id);
    bin_prefix_id = hex_to_chararray(prefix_id);
    
    ba->publish_info(bin_id, bin_prefix_id, NODE_LOCAL, NULL);
    //ba->publish_info(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL);
    //ba->publish_info(bin_id, bin_prefix_id, LINK_LOCAL, (char *) bin_LID._data);
    
    sleep(2);
    pthread_join(event_listener, NULL);
    cout << "disconnecting" << endl;
    sleep(1);
    ba->disconnect();
    free(payload);
    free(end_payload);
    return 0;
}
