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

static int testcounter = 0;

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

int getEvent() {

}

void *event_listener_loop(void *arg) {
    Blackadder *ba = (Blackadder *) arg;
    while (true) {
        Event ev = ba->getEvent();
        if (ev.type == PUBLISHED_DATA) {
            //cout<<"received data of size: "<< (total_buf_size - (sizeof (struct nlmsghdr) + sizeof (unsigned char) + sizeof (unsigned char) + ((int) id_len) * PURSUIT_ID_LEN)) << endl;
            if (ev.data[0] == 'A') {
                if (experiment_started == false) {
                    experiment_started = true;
                    gettimeofday(&start_tv, &tz);
                    printf("START TIME: %ld,%ld \n", start_tv.tv_sec, start_tv.tv_usec);
                }
                //cout << "received published data for ID " << chararray_to_hex(str_id) << "!! size: " << (total_buf_size - (sizeof (struct nlmsghdr) + sizeof (unsigned char) + sizeof (unsigned char) +str_id.size())) << endl;
                counter++;
            }
            if (ev.data[0] == 'B') {
                //printf("Received %d packets\n", counter);
                gettimeofday(&end_tv, &tz);
                //printf("END TIME: %ld,%ld \n", end_tv.tv_sec, end_tv.tv_usec);
                duration.tv_sec = end_tv.tv_sec - start_tv.tv_sec;
                if (end_tv.tv_usec - start_tv.tv_usec > 0) {
                    duration.tv_usec = end_tv.tv_usec - start_tv.tv_usec;
                } else {
                    duration.tv_usec = end_tv.tv_usec + 1000000 - start_tv.tv_usec;
                    duration.tv_sec--;
                }
                printf("duration: %ld seconds and %d microseconds\n\n", duration.tv_sec, duration.tv_usec);
                float left = counter * ((float) payload_size / (float) (1024 * 1024));
                float right = ((float) ((duration.tv_sec * 1000000) + duration.tv_usec)) / 1000000;

                cout << "counter: " << counter << endl;
                cout << "payload_size: " << payload_size << endl;
                //cout << "left: " << left << endl;
                //cout << "right: " << right << endl;

                float throughput = (left / right);

                printf("Throughput: %f MB/sec \n\n", throughput);
                break;
            }
        }
    }
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
    int ret;
    (void) signal(SIGINT, sigfun);

    //string link_local_LID("0000000000000000000000000000000000000000000000000000000000000010");
    //Bitvector bin_LID = Bitvector(link_local_LID);

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
    string id = "1111111111111111";
    string prefix_id = string();
    string LID = string();
    string bin_id = hex_to_chararray(id);
    string bin_prefix_id = hex_to_chararray(prefix_id);

    pthread_create(&event_listener, NULL, event_listener_loop, (void *) ba);

    ba->subscribe_scope(bin_id, prefix_id, DOMAIN_LOCAL, NULL);
    //ba->subscribe_scope(bin_id, prefix_id, LINK_LOCAL, (char *) bin_LID._data);

    pthread_join(event_listener, NULL);
    sleep(1);
    ba->disconnect();
    free(payload);
    free(end_payload);
    return 0;
}
