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
void find_next(string &fragment_id);
void find_previous(string &fragment_id);
void *publisher_loop(void *arg);
void *retransmitter_loop(void *arg);
void *event_listener_loop(void *arg);

int counter = 0;
struct timeval start_tv;
struct timeval end_tv;
struct timeval duration;
int data_len = 1024 * 1024 * 1024;
int fragment_size = 1420;
char *payload = (char *) malloc(fragment_size);
bool experiment_started = false;

string LID = "0000000000000000000000000000000000000000000000000000000000000000";
Bitvector bin_LID = Bitvector(LID);

string hex_root_scope = "0000000000000000";
string hex_zero_id = "0000000000000000";
string hex_info_id = "1111111111111111";
string hex_alg_scope2 = "2222222222222222";
string hex_alg_scope3 = "3333333333333333";

string root_scope = hex_to_chararray(hex_root_scope);
string zero_id = hex_to_chararray(hex_zero_id);
string info_id = hex_to_chararray(hex_info_id);
string alg_scope2 = hex_to_chararray(hex_alg_scope2);
string alg_scope3 = hex_to_chararray(hex_alg_scope3);

pthread_mutex_t retransmission_queue_mutex;
pthread_cond_t retransmission_queue_cond;

map<string, pair<string, bool> > retransmission_channels;

queue<RetransmissionRequest *> retransmission_request_queue;

Blackadder *ba;

class RetransmissionRequest {
public:
    string first_fragment;
    int number_of_fragments;
    string p_to_s;
};

struct publisher_data {
    string id;
    int number_of_fragments;
    char *data;
    Blackadder *ba;
};

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

void sigfun(int sig) {
    (void) signal(SIGINT, SIG_DFL);
    free(payload);
    ba->disconnect();
    delete ba;
    exit(0);
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

void *publisher_loop(void *arg) {
    int test = 0;
    struct publisher_data *pd = (struct publisher_data *) arg;
    cout << "I will now publish all " << pd->number_of_fragments << " fragments for ID " << chararray_to_hex(pd->id) << endl;
    string fragment_id = pd->id + zero_id;
    for (int i = 0; i < pd->number_of_fragments; i++) {
        //cout << "transmitted " << chararray_to_hex(fragment_id) << endl;
        pd->ba->publish_data(fragment_id, PUBLISH_NOW, (char *) bin_LID._data, payload, fragment_size);
        find_next(fragment_id);
        test++;
        if (test % 100 == 0) {
            usleep(100);
        }
    }
    pthread_exit(NULL);
}

void *retransmitter_loop(void *arg) {
    Blackadder *ba = (Blackadder *) arg;
    int test = 0;
    while (true) {
        pthread_mutex_lock(&retransmission_queue_mutex);
        while (retransmission_request_queue.size() > 0) {
            RetransmissionRequest *rr = retransmission_request_queue.front();
            retransmission_request_queue.pop();
            /*retransmit the fragments*/
            //cout << "I will retransmit " << rr->number_of_fragments << " fragments" << " to "<< chararray_to_hex(rr->p_to_s) << endl;
            for (int i = 0; i < rr->number_of_fragments; i++) {
                //cout << "I retransmitted " << chararray_to_hex(rr->first_fragment) << endl;
                string id_to_use = rr->p_to_s + rr->first_fragment.substr(rr->first_fragment.length() - PURSUIT_ID_LEN);
                ba->publish_data(id_to_use, PUBLISH_NOW, (char *) bin_LID._data, payload, fragment_size);
                find_next(rr->first_fragment);
                test++;
                if (test % 100 == 0) {
                    usleep(100);
                }
            }
            delete rr;
            /**************************/
        }
        /*wait for the condition*/
        pthread_cond_wait(&retransmission_queue_cond, &retransmission_queue_mutex);
        pthread_mutex_unlock(&retransmission_queue_mutex);
    }
}

void *event_listener_loop(void *arg) {
    Blackadder *ba = (Blackadder *) arg;
    Bitvector bin_LID = Bitvector(LID);
    string hex_id = string();

    while (true) {
        Event ev = ba->getEvent();
        hex_id = chararray_to_hex(ev.id);
        switch (ev.type) {
            case SCOPE_PUBLISHED:
                cout << "SCOPE_PUBLISHED: " << hex_id << endl;
                break;
            case SCOPE_UNPUBLISHED:
                cout << "SCOPE_UNPUBLISHED: " << hex_id << endl;
                break;
            case START_PUBLISH:
                cout << "START_PUBLISH: " << hex_id << endl;
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

                    usleep(10000);
                    /*open a new thread and start publishing the data*/
                    struct publisher_data pd;
                    pd.id = root_scope + info_id;
                    if (data_len % fragment_size == 0) {
                        pd.number_of_fragments = data_len / fragment_size;
                    } else {
                        pd.number_of_fragments = (data_len / fragment_size) + 1;
                    }
                    pd.data = payload;
                    pd.ba = ba;
                    pthread_t publisher;
                    pthread_create(&publisher, NULL, publisher_loop, &pd);
                    /*************************************************/

                } else {
                    string p_to_s = ev.id;
                    string s_to_p = root_scope + alg_scope2 + p_to_s.substr(p_to_s.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                    map<string, pair<string, bool> >::iterator it = retransmission_channels.find(s_to_p);
                    if (it != retransmission_channels.end()) {
                        if ((*it).second.second == false) {
                            cout << "now I have a path back for sending retransmissions using the ID " << chararray_to_hex(p_to_s) << endl;
                            (*it).second.second == true;
                        }
                    }
                }
                break;
            case STOP_PUBLISH:
                //cout << "STOP_PUBLISH: " << hex_id << endl;
                break;
            case PUBLISHED_DATA:
                //cout << "PUBLISHED_DATA: " << hex_id << endl;
                if (ev.id.compare(0, 2 * PURSUIT_ID_LEN, root_scope + alg_scope2) == 0) {
                    string s_to_p = ev.id;
                    string p_to_s = root_scope + alg_scope3 + s_to_p.substr(s_to_p.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                    if (retransmission_channels.find(s_to_p) != retransmission_channels.end()) {
                        /****************/
                        unsigned char IDlen;
                        int number_of_fragments;
                        memcpy(&IDlen, ev.data, sizeof (IDlen));
                        string ID_to_retransmit = string(ev.data + sizeof (IDlen), IDlen * PURSUIT_ID_LEN);
                        memcpy(&number_of_fragments, ev.data + sizeof (IDlen) + IDlen * PURSUIT_ID_LEN, sizeof (int));
                        int sequence_number;
                        memcpy(&sequence_number, ev.data + sizeof (IDlen) + IDlen * PURSUIT_ID_LEN + sizeof (int), sizeof (sequence_number));
                        //cout << "retransmission request for " << chararray_to_hex(ID_to_retransmit) << ", number of fragments: " << number_of_fragments << ", sequence number: " << sequence_number << endl;
                        RetransmissionRequest *rr = new RetransmissionRequest();
                        rr->first_fragment = ID_to_retransmit;
                        rr->number_of_fragments = number_of_fragments;
                        rr->p_to_s = p_to_s;
                        pthread_mutex_lock(&retransmission_queue_mutex);
                        retransmission_request_queue.push(rr);
                        pthread_cond_signal(&retransmission_queue_cond);
                        pthread_mutex_unlock(&retransmission_queue_mutex);
                        /****************/
                    } else {
                        /*that's the first response from a subscriber*/
                        /*I will advertise a channel so that I can send retransmissions to the subscriber*/
                        cout << "whenever a retransmission request is published to channel " << chararray_to_hex(s_to_p) << ", I will retransmit the fragment to " << chararray_to_hex(p_to_s) << endl;
                        retransmission_channels.insert(pair<string, pair<string, bool > >(s_to_p, pair<string, bool >(p_to_s, false)));
                        string bin_id = p_to_s.substr(p_to_s.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                        string bin_prefix_id = p_to_s.substr(0, p_to_s.length() - PURSUIT_ID_LEN);
                        ba->publish_info(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL);
                    }
                }
                break;
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    pthread_t event_listener, retransmitter;
    (void) signal(SIGINT, sigfun);

    pthread_mutex_init(&retransmission_queue_mutex, NULL);
    pthread_cond_init(&retransmission_queue_cond, NULL);

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
    pthread_create(&retransmitter, NULL, retransmitter_loop, (void *) ba);
    /*****************************publish root scope /0000000000000000 ************************************/
    string null_str = string();
    ba->publish_scope(root_scope, null_str, DOMAIN_LOCAL, NULL);
    /*publish information item /0000000000000000/1111111111111111 - This is the Actual Item I want to send*/
    ba->publish_info(info_id, root_scope, DOMAIN_LOCAL, NULL);
    /******************************************************************************************************/
    /*subscribe to the scope with the algorithmically calculated ID /0000000000000000/ALGID(1111111111111111)
     * for now, ALGID(1111111111111111)=2222222222222222 :)
    this scope will be used by the subscribers to send retransmission requests*/
    ba->subscribe_scope(alg_scope2, root_scope, DOMAIN_LOCAL, NULL);
    /*publish scope with the algorithmically calculated ID /0000000000000000/ALGID(ALGID(1111111111111111))
     * for now, ALGID(ALGID(1111111111111111))=3333333333333333 :)*/
    ba->publish_scope(alg_scope3, root_scope, DOMAIN_LOCAL, NULL);
    pthread_join(event_listener, NULL);
    return 0;
}
