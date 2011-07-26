#include "blackadder.hpp"
#include <signal.h>

int counter = 0;
struct timeval start_tv;
struct timeval end_tv;
struct timeval duration;
int payload_size = 1250;
char *payload = (char *) malloc(payload_size);
char *end_payload = (char *) malloc(payload_size);
bool experiment_started = false;

Blackadder *ba;

using namespace std;

void sigfun(int sig) {

    (void) signal(SIGINT, SIG_DFL);
    free(payload);
    free(end_payload);
    ba->disconnect();
    delete ba;
    exit(0);
}

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
    string id = "0000000000000000";
    string prefix_id = string();
    string bin_id = hex_to_chararray(id);
    string bin_prefix_id = hex_to_chararray(prefix_id);
    ba->subscribe_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL);
    while (true) {
        Event ev = ba->getEvent();
        switch (ev.type) {
            case SCOPE_PUBLISHED:
                cout << "SCOPE_PUBLISHED: " << chararray_to_hex(ev.id) << endl;
                bin_id = ev.id.substr(ev.id.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                bin_prefix_id = ev.id.substr(0, ev.id.length() - PURSUIT_ID_LEN);
                ba->subscribe_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL);
                break;
            case SCOPE_UNPUBLISHED:
                cout << "SCOPE_UNPUBLISHED: " << chararray_to_hex(ev.id) << endl;
                break;
            case START_PUBLISH:
                cout << "START_PUBLISH: " << chararray_to_hex(ev.id) << endl;
                break;
            case STOP_PUBLISH:
                cout << "STOP_PUBLISH: " << chararray_to_hex(ev.id) << endl;
                break;
            case PUBLISHED_DATA:
                cout << "PUBLISHED_DATA: " << chararray_to_hex(ev.id) << endl;
                cout << "data size: " << ev.data_len << endl;
                break;
        }
    }
    free(payload);
    free(end_payload);
    ba->disconnect();
    delete ba;
    return 0;
}
