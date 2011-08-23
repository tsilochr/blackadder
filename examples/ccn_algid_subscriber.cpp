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
#include <vector>
#include <math.h>
#include <arpa/inet.h>
#include <sys/time.h>

using namespace std;

class ExpectedFragmentSequence;
string hex_to_chararray(string const& hexstr);
string chararray_to_hex(const string &str);
void sigfun(int sig);
void *event_listener_loop(void *arg);
void *timeout_handler(void *arg);
void *subscriber_handler(void *arg);

int global_number_of_fragments = 0;
int counter = 0;
struct timezone tz;
struct timeval start_tv;
struct timeval end_tv;
struct timeval duration;
int payload_size = 1420;
char *payload = (char *) malloc(payload_size);
bool experiment_started = false;

Blackadder *ba;
ExpectedFragmentSequence *efs;



string hex_root_scope = "0000000000000000";
string hex_zero_id = "0000000000000000";
string hex_info_id = "1111111111111111";
string hex_alg_scope2 = "2222222222222222";

string root_scope = hex_to_chararray(hex_root_scope);
string zero_id = hex_to_chararray(hex_zero_id);
string info_id = hex_to_chararray(hex_info_id);
string alg_scope2 = hex_to_chararray(hex_alg_scope2);
string full_info_id = root_scope + info_id;

string hex_id;

pthread_t timeout_thread;
pthread_t subscriber_thread;
pthread_t event_listener;

pthread_mutex_t global_mutex;

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

/*very very bad implementation*/
void calculate_fragment_from_int(string &fragment_id, int offset) {
    for (int i = 0; i < offset; i++) {
        find_next(fragment_id);
    }
}

int calculate_number_of_fragments(string &start_id, string &end_id) {
    int result = 1;
    if (start_id.compare(end_id) > 0) {
        return -1;
    }
    for (string::size_type i = 0; i < start_id.size(); i++) {
        unsigned short first = (unsigned short) (unsigned char) start_id[i];
        unsigned short last = (unsigned short) (unsigned char) end_id[i];
        result = result + pow(256, start_id.size() - i - 1)*(last - first);
    }
    return result;
}

void create_random_ID(string &id) {
    static const char alphanum[] =
            "0123456789"
            "abcdef";

    for (int i = 0; i < 2 * PURSUIT_ID_LEN; i++) {
        id = id + alphanum[rand() % (sizeof (alphanum) - 1)];
    }
}

class ExpectedFragmentSequence {
public:
    Bitvector fragments_map;
    int number_of_fragments;
    int fragments_so_far;
    string first_fragment;
    string last_fragment;
    string expected_fragment;
    int expected_fragment_offset;
    string s_to_p_channel;
    string p_to_s_channel;
    int time_beat;

    bool belongsIn(string &id) {
        if ((id.compare(first_fragment) >= 0) && (id.compare(last_fragment) <= 0)) {
            //cout<<chararray_to_hex(id)<< " belongs between "<<chararray_to_hex(first_fragment)<< "  and  " <<chararray_to_hex(last_fragment)<<endl;
            return true;
        } else {
            //cout<<chararray_to_hex(id)<< " DOES NOT BELONG between "<<chararray_to_hex(first_fragment)<< "  and  " <<chararray_to_hex(last_fragment)<<endl;
            return false;
        }
    }

    void printEFS() {
        cout << "****** expected fragment sequence********" << endl;
        cout << "efs->number_of_fragments  " << number_of_fragments << endl;
        cout << "efs->fragments_so_far  " << fragments_so_far << endl;
        cout << "efs->first_fragment  " << chararray_to_hex(first_fragment) << endl;
        cout << "efs->last_fragment  " << chararray_to_hex(last_fragment) << endl;
        cout << "efs->expected_fragment  " << chararray_to_hex(expected_fragment) << endl;
        cout << "efs->s_to_p_channel  " << chararray_to_hex(s_to_p_channel) << endl;
        cout << "efs->p_to_s_channel  " << chararray_to_hex(p_to_s_channel) << endl;
    }
};

void printResult() {
    cout << "received ALL fragments " << endl;
    gettimeofday(&end_tv, &tz);
    printf("END TIME: %ld,%ld \n", end_tv.tv_sec, end_tv.tv_usec);
    duration.tv_sec = end_tv.tv_sec - start_tv.tv_sec;
    if (end_tv.tv_usec - start_tv.tv_usec > 0) {
        duration.tv_usec = end_tv.tv_usec - start_tv.tv_usec;
    } else {
        duration.tv_usec = end_tv.tv_usec + 1000000 - start_tv.tv_usec;
        duration.tv_sec--;
    }
    printf("duration: %ld seconds and %d microseconds\n\n", duration.tv_sec, duration.tv_usec);
    float left = global_number_of_fragments * ((float) payload_size / (float) (1024 * 1024));
    float right = ((float) ((duration.tv_sec * 1000000) + duration.tv_usec)) / 1000000;
    cout << "counter: " << global_number_of_fragments << endl;
    cout << "payload_size: " << payload_size << endl;
    float throughput = (left / right);
    printf("Throughput: %f MB/sec \n\n", throughput);
    cout << "THAT'S ALL FOLKS" << endl;
}

void sigfun(int sig) {
    (void) signal(SIGINT, SIG_DFL);
    free(payload);
    ba->disconnect();
    delete ba;
    pthread_cancel(event_listener);
    exit(0);
}

void *subscriber_handler(void *arg) {
    int test = 0;
    Blackadder *ba = (Blackadder *) arg;
    zero_id = hex_to_chararray("0000000000000000");
    cout << "I should subscribe to " << global_number_of_fragments << " fragments" << endl;
    for (int i = 0; i < global_number_of_fragments; i++) {
        ba->subscribe_info(zero_id, alg_scope2, DOMAIN_LOCAL, NULL);
        find_next(zero_id);
        test++;
        if(test%10 == 0) {
            usleep(10000);
        }
    }
}

void *timeout_handler(void *arg) {
    Blackadder *ba = (Blackadder *) arg;
    while (true) {
        pthread_mutex_lock(&global_mutex);
        if (efs == NULL) {
            cout << "NO EFS - NULL - NADA" << endl;
            break;
        }
        string retr_id = efs->expected_fragment.substr(efs->expected_fragment.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
        efs->time_beat--;
        //cout << efs->time_beat << endl;
        if (efs->time_beat == 0) {
            cout << "timeout" << endl;
            /*find all non received fragments and send multiple requests for sequences of fragments*/
            int i = efs->expected_fragment_offset;
            //cout << "efs->expected_fragment_offset: " << efs->expected_fragment_offset << endl;
            //cout << "efs->fragments_map.size()" << efs->fragments_map.size() << endl;
            while (i < efs->fragments_map.size()) {
                if (Bitvector::Bit::unspecified_bool_type(efs->fragments_map[i]) == false) {
                    //cout << "requesting again by unsubscribing and subscribing " << chararray_to_hex(alg_scope2 + retr_id) << endl;
                    ba->unsubscribe_info(retr_id, alg_scope2, DOMAIN_LOCAL, NULL);
                    ba->subscribe_info(retr_id, alg_scope2, DOMAIN_LOCAL, NULL);
                }
                find_next(retr_id);
                i++;
            }
            //cout << "end of timeout" << endl;
            efs->time_beat = 5;
        } else if (efs->time_beat < 0) {
            cout << "that should not happen" << endl;
        }

        pthread_mutex_unlock(&global_mutex);
        usleep(100000);
    }
}

void *event_listener_loop(void *arg) {
    Blackadder *ba = (Blackadder *) arg;
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
                break;
            case STOP_PUBLISH:
                cout << "STOP_PUBLISH: " << hex_id << endl;
                break;
            case PUBLISHED_DATA:
                //cout << "PUBLISHED_DATA: " << hex_id << endl;
                if (ev.id.compare(full_info_id) == 0) {
                    /*start measuring*/
                    gettimeofday(&start_tv, &tz);
                    printf("START TIME: %ld,%ld \n", start_tv.tv_sec, start_tv.tv_usec);
                    /*that's the metadata*/
                    memcpy(&global_number_of_fragments, ev.data, sizeof (global_number_of_fragments));
                    efs = new ExpectedFragmentSequence();
                    /*set up the required state for defragmentation*/
                    efs->number_of_fragments = global_number_of_fragments;
                    efs->fragments_so_far = 0;
                    string zero_id = hex_to_chararray("0000000000000000");
                    efs->expected_fragment = alg_scope2 + zero_id;
                    cout << "expected fragment: " << chararray_to_hex(efs->expected_fragment = alg_scope2 + zero_id) << endl;
                    efs->first_fragment = ev.id + zero_id;
                    efs->last_fragment = efs->first_fragment;
                    efs->expected_fragment_offset = 0;
                    efs->fragments_map = Bitvector(global_number_of_fragments);
                    efs->time_beat = 5;
                    calculate_fragment_from_int(efs->last_fragment, global_number_of_fragments);
                    efs->printEFS();
                    pthread_create(&subscriber_thread, NULL, subscriber_handler, (void *) ba);
                    pthread_create(&timeout_thread, NULL, timeout_handler, (void *) ba);
                } else {
                    /*a fragment has arrived - welcome!*/
                    if (efs != NULL) {
                        //cout << "received fragment " << chararray_to_hex(ev.id) << endl;
                        pthread_mutex_lock(&global_mutex);
                        efs->time_beat = 5;
                        if (ev.id.compare(efs->expected_fragment) == 0) {
                            //cout << "expected" << endl;
                            efs->fragments_so_far++;
                            efs->fragments_map[efs->expected_fragment_offset] = true;
                            //cout << "setting bit " << (efs->expected_fragment_offset) << endl;
                            //cout << efs->fragments_map.to_string().c_str() << endl;
                            efs->expected_fragment_offset++;
                            find_next(efs->expected_fragment);
                            string unsubscribe_id = ev.id.substr(ev.id.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                            ba->unsubscribe_info(unsubscribe_id, alg_scope2, DOMAIN_LOCAL, NULL);
                            if (efs->fragments_so_far == efs->number_of_fragments) {
                                pthread_cancel(timeout_thread);
                                delete efs;
                                efs = NULL;
                                printResult();
                            }
                        } else {
                            //cout << "NOT expected" << endl;
                            int distance = calculate_number_of_fragments(efs->expected_fragment, ev.id) - 1; //e.g. from 0 to 1 distance = 1
                            if (distance == -1) {
                                cout << chararray_to_hex(ev.id) << " was a duplicate" << endl;
                                break;
                            }
                            if (Bitvector::Bit::unspecified_bool_type(efs->fragments_map[efs->expected_fragment_offset + distance]) != false) {
                                cout << "a duplicate...I am unsubscribing just in case" << endl;
                            } else {
                                efs->fragments_map[efs->expected_fragment_offset + distance] = true;
                                //cout << "setting bit " << (efs->expected_fragment_offset + distance) << endl;
                                //cout << efs->fragments_map.to_string().c_str() << endl;
                                efs->fragments_so_far++;
                            }
                            string unsubscribe_id = ev.id.substr(ev.id.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                            ba->unsubscribe_info(unsubscribe_id, alg_scope2, DOMAIN_LOCAL, NULL);

                            if (efs->fragments_so_far == efs->number_of_fragments) {
                                pthread_cancel(timeout_thread);
                                delete efs;
                                efs = NULL;
                                printResult();
                            }
                        }
                        pthread_mutex_unlock(&global_mutex);
                    }
                }
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    int ret;
    (void) signal(SIGINT, sigfun);
    pthread_mutex_init(&global_mutex, NULL);
    srand(time(NULL));
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
    ret = pthread_create(&event_listener, NULL, event_listener_loop, (void *) ba);
    string null_str = string();
    ba->subscribe_scope(root_scope, null_str, DOMAIN_LOCAL, NULL);
    pthread_join(event_listener, NULL);
    return 0;
}
