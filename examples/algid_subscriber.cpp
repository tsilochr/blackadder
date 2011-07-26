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
bool belongsIn(string &first, string &last, string &to_check);
void find_next(string &fragment_id);
void find_previous(string &fragment_id);
void calculate_fragment_from_int(string &fragment_id, int offset);
int calculate_number_of_fragments(string &start_id, string &end_id);
void create_random_ID(string &id);
void sigfun(int sig);
void *timeout_handler(void *arg);
void *event_listener_loop(void *arg);

int sequence_number = 0;

int global_number_of_fragments = 0;
int retransmission_threshold;


int counter = 0;
struct timezone tz;
struct timeval start_tv;
struct timeval end_tv;
struct timeval duration;
int payload_size = 1420;
char *payload = (char *) malloc(payload_size);
char *end_payload = (char *) malloc(payload_size);
bool experiment_started = false;

Blackadder *ba;

ExpectedFragmentSequence *efs;

pthread_t event_listener;
pthread_t timeout_thread;

pthread_mutex_t global_mutex;
pthread_cond_t global_cond;

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
string full_info_id = root_scope + info_id;


string backchannel_id; /*in HEX!!*/

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

bool belongsIn(string &first, string &last, string &to_check) {
    if ((to_check.compare(first) >= 0) && (to_check.compare(last) <= 0)) {
        //cout<<chararray_to_hex(id)<< " belongs between "<<chararray_to_hex(first_fragment)<< "  and  " <<chararray_to_hex(last_fragment)<<endl;
        return true;
    } else {
        //cout<<chararray_to_hex(id)<< " DOES NOT BELONG between "<<chararray_to_hex(first_fragment)<< "  and  " <<chararray_to_hex(last_fragment)<<endl;
        return false;
    }
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
    for (int i = 0; i < offset - 1; i++) {
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

void sigfun(int sig) {
    (void) signal(SIGINT, SIG_DFL);
    free(payload);
    free(end_payload);
    ba->disconnect();
    delete ba;
    pthread_cancel(event_listener);
    exit(0);
}

void *timeout_handler(void *arg) {
    Blackadder *ba = (Blackadder *) arg;
    while (true) {
        int retr_thres_counter = 0;
        pthread_mutex_lock(&global_mutex);
        if (efs == NULL) {
            cout << "NO EFS - NULL - NADA" << endl;
            break;
        }
        string temp_id = efs->expected_fragment;
        efs->time_beat--;
        if (efs->time_beat == 0) {
            cout << "timeout" << endl;
            /*find all non received fragments and send multiple requests for sequences of fragments*/
            int i = efs->expected_fragment_offset;
            //cout << "efs->expected_fragment_offset: " << efs->expected_fragment_offset << endl;
            while (i < efs->fragments_map.size()) {
                int temp_offset = 0;
                int required_fragments = 0;
                while ((Bitvector::Bit::unspecified_bool_type(efs->fragments_map[i]) == false) && (i < efs->fragments_map.size())) {
                    required_fragments++;
                    i++;
                    temp_offset++;
                    retr_thres_counter++;
                }
                pthread_mutex_unlock(&global_mutex);
                string test = temp_id;
                calculate_fragment_from_int(test, required_fragments);
                //cout << "request retransmission " << chararray_to_hex(temp_id) << " to  " << chararray_to_hex(test) << "  " << required_fragments << endl;
                unsigned char IDlen = temp_id.length() / PURSUIT_ID_LEN;
                char *retransmission_payload = (char *) malloc(sizeof (IDlen) + temp_id.length() + sizeof (required_fragments) + sizeof (sequence_number));
                memcpy(retransmission_payload, &IDlen, sizeof (IDlen));
                memcpy(retransmission_payload + sizeof (IDlen), temp_id.c_str(), temp_id.length());
                memcpy(retransmission_payload + sizeof (IDlen) + temp_id.length(), &required_fragments, sizeof (required_fragments));
                memcpy(retransmission_payload + sizeof (IDlen) + temp_id.length() + sizeof (required_fragments), &sequence_number, sizeof (sequence_number));
                ba->publish_data(efs->s_to_p_channel, DOMAIN_LOCAL, NULL, retransmission_payload, sizeof (IDlen) + temp_id.length() + sizeof (required_fragments) + sizeof (sequence_number));
                sequence_number++;
                free(retransmission_payload);
                if (retr_thres_counter > retransmission_threshold) {
                    //cout << "breaking because of threshold " << retransmission_threshold << endl;
                    break;
                }

                calculate_fragment_from_int(temp_id, temp_offset);
                find_next(temp_id);
                pthread_mutex_lock(&global_mutex);

                while ((Bitvector::Bit::unspecified_bool_type(efs->fragments_map[i]) != false) && (i < efs->fragments_map.size())) {
                    //cout << chararray_to_hex(temp_id) << "  is set" << endl;
                    find_next(temp_id);
                    i++;
                    temp_offset++;
                }
                temp_offset = temp_offset++;
            }
            efs->time_beat = 10;
        } else if (efs->time_beat < 0) {
            cout << "that should not happen" << endl;
        }

        pthread_mutex_unlock(&global_mutex);
        usleep(20000);

        //cout << "end of timeout!" << endl;
    }
}

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

void *event_listener_loop(void *arg) {
    int bug_counter = 0;
    Blackadder *ba = (Blackadder *) arg;
    string bin_prefix_id = string();
    string hex_id = string();
    string bin_backchannel_id = string();
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
                bin_backchannel_id = hex_to_chararray(backchannel_id);
                if (hex_id.compare("00000000000000002222222222222222" + backchannel_id) == 0) {
                    char *hello = (char *) malloc(sizeof ("hello"));
                    memcpy(hello, "hello", sizeof ("hello"));
                    ba->publish_data(ev.id, DOMAIN_LOCAL, NULL, hello, sizeof ("hello"));
                    cout << hex_id << " will be used as the backchannel to the publisher" << endl;
                }
                break;
            case STOP_PUBLISH:
                cout << "STOP_PUBLISH: " << hex_id << endl;
                break;
            case PUBLISHED_DATA:
                //cout << "PUBLISHED_DATA: " << hex_id << endl;
                //cerr<<"lock!!!"<<endl;
                if (ev.id.compare(full_info_id) == 0) {
                    /*start measuring*/
                    gettimeofday(&start_tv, &tz);
                    printf("START TIME: %ld,%ld \n", start_tv.tv_sec, start_tv.tv_usec);
                    /*that's the metadata*/
                    efs = new ExpectedFragmentSequence();
                    memcpy(&global_number_of_fragments, ev.data, sizeof (global_number_of_fragments));

                    /*the bandwidth would be useful to calculate that as well :)*/
                    retransmission_threshold = global_number_of_fragments / 1;

                    /*set up the required state for defragmentation*/
                    efs->number_of_fragments = global_number_of_fragments;
                    efs->fragments_so_far = 0;
                    string zero_id = hex_to_chararray("0000000000000000");
                    efs->expected_fragment = ev.id + zero_id;
                    efs->first_fragment = ev.id + zero_id;
                    efs->last_fragment = efs->first_fragment;
                    calculate_fragment_from_int(efs->last_fragment, global_number_of_fragments);
                    efs->expected_fragment_offset = 0;
                    efs->fragments_map = Bitvector(global_number_of_fragments);
                    efs->time_beat = 10;
                    /*advertised the item /0000000000000000/ALGID(1111111111111111)/alg(ID,myself)
                     * (for now, ALGID(1111111111111111)=2222222222222222 and alg(ID,myself)=3333333333333333) to create the back channel to the publisher*/
                    string prefix_id = "00000000000000003333333333333333";
                    backchannel_id = string();
                    create_random_ID(backchannel_id); /*in hex*/
                    efs->p_to_s_channel = hex_to_chararray(prefix_id + backchannel_id);
                    bin_prefix_id = hex_to_chararray(prefix_id);
                    bin_backchannel_id = hex_to_chararray(backchannel_id);
                    ba->subscribe_info(bin_backchannel_id, bin_prefix_id, DOMAIN_LOCAL, NULL);
                    prefix_id = "00000000000000002222222222222222";
                    efs->s_to_p_channel = hex_to_chararray(prefix_id + backchannel_id);
                    bin_prefix_id = hex_to_chararray(prefix_id);
                    bin_backchannel_id = hex_to_chararray(backchannel_id);
                    ba->publish_info(bin_backchannel_id, bin_prefix_id, DOMAIN_LOCAL, NULL);
                    efs->printEFS();
                    pthread_create(&timeout_thread, NULL, timeout_handler, (void *) ba);
                } else {
                    pthread_mutex_lock(&global_mutex);
                    if (efs != NULL) {
                        efs->time_beat = 10;
                        if (ev.id.substr(0, 2 * PURSUIT_ID_LEN).compare(full_info_id) == 0) {
                            //cout << "an in-band transmission: " << chararray_to_hex(ev.id) << endl;
                            if (ev.id.compare(efs->expected_fragment) == 0) {
                                //cout << "expected" << chararray_to_hex(ev.id) << endl;
                                find_next(efs->expected_fragment);
                                efs->fragments_so_far++;
                                efs->fragments_map[efs->expected_fragment_offset] = true;
                                //cout << "setting bit " << (efs->expected_fragment_offset) << endl;
                                //cout << efs->fragments_map.to_string().c_str() << endl;
                                efs->expected_fragment_offset++;
                                if (efs->fragments_so_far == efs->number_of_fragments) {
                                    pthread_cancel(timeout_thread);
                                    delete efs;
                                    efs = NULL;
                                    printResult();
                                    break;
                                }
                            } else {
                                //cout << "NOT expected" << endl;
                                int distance = calculate_number_of_fragments(efs->expected_fragment, ev.id) - 1; //e.g. from 0 to 1 distance = 1
                                if (distance == -1) {
                                    cout << chararray_to_hex(ev.id) << " is a duplicate " << endl;
                                } else if (Bitvector::Bit::unspecified_bool_type(efs->fragments_map[efs->expected_fragment_offset + distance]) != false) {
                                    cout << chararray_to_hex(ev.id) << " is a duplicate! " << endl;
                                } else {
                                    efs->fragments_so_far++;
                                    efs->fragments_map[efs->expected_fragment_offset + distance] = true;
                                    //cout << "setting bit " << (efs->expected_fragment_offset + distance) << endl;
                                    //cout << efs->fragments_map.to_string().c_str() << endl;
                                }
                                if (efs->fragments_so_far == efs->number_of_fragments) {
                                    pthread_cancel(timeout_thread);
                                    delete efs;
                                    efs = NULL;
                                    printResult();
                                    break;
                                }
                            }
                        } else {
                            string mapped_id = root_scope + info_id + ev.id.substr(ev.id.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                            //cout << "a retransmission: " << chararray_to_hex(mapped_id) << endl;
                            if (mapped_id.compare(efs->expected_fragment) == 0) {
                                //cout << "expected" << endl;
                                efs->fragments_so_far++;
                                efs->fragments_map[efs->expected_fragment_offset] = true;
                                if (efs->fragments_so_far == efs->number_of_fragments) {
                                    pthread_cancel(timeout_thread);
                                    delete efs;
                                    efs = NULL;
                                    printResult();
                                    break;
                                }
                                if (Bitvector::Bit::unspecified_bool_type(efs->fragments_map[efs->expected_fragment_offset + 1]) == false) {
                                    find_next(efs->expected_fragment);
                                    //cout << "setting bit " << (efs->expected_fragment_offset) << endl;
                                    //cout << efs->fragments_map.to_string().c_str() << endl;
                                    efs->expected_fragment_offset++;
                                } else {
                                    //cout << "the next fragment is already received so I have to move to the new expected" << endl;
                                    //cout << "setting bit " << (efs->expected_fragment_offset) << endl;
                                    //cout << efs->fragments_map.to_string().c_str() << endl;
                                    find_next(efs->expected_fragment);
                                    efs->expected_fragment_offset++;
                                    while ((Bitvector::Bit::unspecified_bool_type(efs->fragments_map[efs->expected_fragment_offset]) != false) && (efs->expected_fragment_offset < efs->fragments_map.size())) {
                                        find_next(efs->expected_fragment);
                                        efs->expected_fragment_offset++;
                                    }
                                    //cout << "now I am expecting " << chararray_to_hex(efs->expected_fragment) << endl;
                                    //cout << "now I am expecting " << efs->expected_fragment_offset << endl;
                                }
                            } else {
                                //cout << "NOT expected" << endl;
                                int distance = calculate_number_of_fragments(efs->expected_fragment, mapped_id) - 1; //e.g. from 0 to 1 distance = 1
                                if (distance == -1) {
                                    cout << chararray_to_hex(mapped_id) << " was a duplicate!!" << endl;
                                } else if (Bitvector::Bit::unspecified_bool_type(efs->fragments_map[efs->expected_fragment_offset + distance]) != false) {
                                    //cout << chararray_to_hex(mapped_id) << " is a duplicate!!! " << endl;
                                } else {
                                    efs->fragments_so_far++;
                                    efs->fragments_map[efs->expected_fragment_offset + distance] = true;
                                    //cout << "setting bit " << (efs->expected_fragment_offset + distance) << endl;
                                    //cout << efs->fragments_map.to_string().c_str() << endl;
                                    if (efs->fragments_so_far == efs->number_of_fragments) {
                                        pthread_cancel(timeout_thread);
                                        delete efs;
                                        efs = NULL;
                                        printResult();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    pthread_mutex_unlock(&global_mutex);
                }
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    int ret;

    (void) signal(SIGINT, sigfun);
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
    pthread_mutex_init(&global_mutex, NULL);
    pthread_cond_init(&global_cond, NULL);
    ret = pthread_create(&event_listener, NULL, event_listener_loop, (void *) ba);
    string null_str = string();
    ba->subscribe_scope(root_scope, null_str, DOMAIN_LOCAL, NULL);
    pthread_join(event_listener, NULL);
    return 0;
}
