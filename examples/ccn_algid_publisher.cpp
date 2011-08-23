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

#include "blackadder.hpp"
#include <signal.h>
#include <pthread.h>
#include <map>
#include <queue>
#include <math.h>

using namespace std;

class RetransmissionRequest;
struct publisher_data;

string hex_to_chararray(string const& hexstr);
string chararray_to_hex(const string &str);
void sigfun(int sig);
void *event_listener_loop(void *arg);

pthread_t event_listener;
pthread_t advertiser;

int data_len = 50 * 1024 * 1024;
int fragment_size = 1420;
char *payload = (char *) malloc(fragment_size);
int number_of_fragments;

string LID = "0000000000000000000000000000000000000000000000000000000000000000";
Bitvector bin_LID = Bitvector(LID);

string hex_root_scope = "0000000000000000";
string hex_zero_id = "0000000000000000";
string hex_info_id = "1111111111111111";
string hex_alg_scope2 = "2222222222222222";

string root_scope = hex_to_chararray(hex_root_scope);
string zero_id = hex_to_chararray(hex_zero_id);
string info_id = hex_to_chararray(hex_info_id);
string alg_scope2 = hex_to_chararray(hex_alg_scope2);


Blackadder *ba;

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

void find_next(string &fragment_id) {
    for (int i = fragment_id.length() - 1; i >= 0; i--) {
        if (fragment_id.at(i) != -1) {
            if (fragment_id.at(i) != 127) {
                fragment_id.at(i)++;
                //cout << "i: " << i << "  fragment.at  " << (int) fragment_id.at(i) << endl;
                break;
            } else {
                fragment_id.at(i) = -128;
                //cout << "i: " << i << "  fragment.at  " << (int) fragment_id.at(i) << endl;
                break;
            }
        } else {
            fragment_id.at(i) = 0;
        }
    }
}

void find_previous(string &fragment_id) {
    for (int i = fragment_id.length() - 1; i >= 0; i--) {
        if (fragment_id.at(i) != 0) {
            if (fragment_id.at(i) != -128) {
                fragment_id.at(i)--;
                //cout << "i: " << i << "  fragment.at  " << (int) fragment_id.at(i) << endl;
                break;
            } else {
                fragment_id.at(i) = 127;
                //cout << "i: " << i << "  fragment.at  " << (int) fragment_id.at(i) << endl;
                break;
            }
        } else {
            fragment_id.at(i) = -1;
        }
    }
}

void sigfun(int sig) {
    (void) signal(SIGINT, SIG_DFL);
    free(payload);
    ba->disconnect();
    delete ba;
    exit(0);
}

void *event_listener_loop(void *arg) {
    Blackadder *ba = (Blackadder *) arg;
    string hex_id = string();
    while (true) {
        Event ev = ba->getEvent();
        hex_id = chararray_to_hex(ev.id);
        switch (ev.type) {
            case SCOPE_PUBLISHED:
                //cout << "SCOPE_PUBLISHED: " << hex_id << endl;
                break;
            case SCOPE_UNPUBLISHED:
                //cout << "SCOPE_UNPUBLISHED: " << hex_id << endl;
                break;
            case START_PUBLISH:
                //cout << "START_PUBLISH: " << hex_id << endl;
                if (ev.id.compare(root_scope + info_id) == 0) {
                    /*a subscriber appeared for that item*/
                    /*publish the "metadata" for this item*/
                    int number_of_fragments;
                    if (data_len % fragment_size == 0) {
                        number_of_fragments = data_len / fragment_size;
                    } else {
                        number_of_fragments = (data_len / fragment_size) + 1;
                    }
                    char *metadata = (char *) malloc(sizeof (number_of_fragments));
                    memcpy(metadata, &number_of_fragments, sizeof (number_of_fragments));
                    ba->publish_data(ev.id, DOMAIN_LOCAL, NULL, metadata, sizeof (number_of_fragments));
                } else {
                    //cout << "publishing fragment " << chararray_to_hex(ev.id) << endl;
                    ba->publish_data(ev.id, DOMAIN_LOCAL, NULL, payload, fragment_size);
                    string prefix = ev.id.substr(0, ev.id.length() - PURSUIT_ID_LEN);
                    string id = ev.id.substr(ev.id.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                    ba->unpublish_info(id, prefix, DOMAIN_LOCAL, NULL);
                }
                break;
            case STOP_PUBLISH:
                //cout << "STOP_PUBLISH: " << hex_id << endl;
                break;
            case PUBLISHED_DATA:
                //cout << "PUBLISHED_DATA: " << hex_id << endl;
                break;
        }
    }
    pthread_exit(NULL);
}

void *advertiser_loop(void *arg) {
    Blackadder *ba = (Blackadder *) arg;

}

int main(int argc, char* argv[]) {
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
    pthread_create(&advertiser, NULL, advertiser_loop, (void *) ba);
    /*****************************publish root scope /0000000000000000 ************************************/
    string null_str = string();
    ba->publish_scope(root_scope, null_str, DOMAIN_LOCAL, NULL);
    /*publish information item /0000000000000000/1111111111111111 - This is the Actual Item I want to send*/
    ba->publish_info(info_id, root_scope, DOMAIN_LOCAL, NULL);
    /******************************************************************************************************/
    ba->publish_scope(alg_scope2, null_str, DOMAIN_LOCAL, NULL);
    /*advertise all fragments under the algorithmic scope*/
    if (data_len % fragment_size == 0) {
        number_of_fragments = data_len / fragment_size;
    } else {
        number_of_fragments = (data_len / fragment_size) + 1;
    }
    for (int i = 0; i < number_of_fragments; i++) {
        ba->publish_info(zero_id, alg_scope2, DOMAIN_LOCAL, NULL);
        find_next(zero_id);
        //cerr << "advertised " << chararray_to_hex(zero_id) << endl;
        usleep(10);
    }
    cout << "advertised all fragments" << endl;
    pthread_join(event_listener, NULL);
    pthread_join(advertiser, NULL);
    return 0;
}
