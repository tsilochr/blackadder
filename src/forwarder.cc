#include "forwarder.hh"
#include <click/glue.hh>

CLICK_DECLS

Forwarder::Forwarder() {

}

Forwarder::~Forwarder() {
}

int Forwarder::configure(Vector<String> &conf, ErrorHandler *errh) {
    int port;
    int reverse_proto;
    gc = (GlobalConf *) cp_element(conf[0], this);
    _id = 0;
    click_chatter("*****************************************************FORWARDER CONFIGURATION*****************************************************");
    click_chatter("Forwarder: internal LID: %s", gc->iLID->to_string().c_str());
    cp_integer(String("0x080a"), 16, &reverse_proto);
    proto_type = htons(reverse_proto);
    if (gc->use_mac == true) {
        cp_integer(conf[1], &number_of_links);
        click_chatter("Forwarder: Number of Links: %d", number_of_links);
        for (int i = 0; i < number_of_links; i++) {
            cp_integer(conf[2 + 4 * i], &port);
            EtherAddress * src = new EtherAddress();
            EtherAddress * dst = new EtherAddress();
            cp_ethernet_address(conf[3 + 4 * i], src, this);
            cp_ethernet_address(conf[4 + 4 * i], dst, this);
            struct forwarding_entry *fe = (struct forwarding_entry *) CLICK_LALLOC(sizeof (struct forwarding_entry));
            fe->src = src;
            fe->dst = dst;
            fe->port = port;
            fe->LID = new BABitvector(FID_LEN * 8);
            for (int j = 0; j < conf[5 + 4 * i].length(); j++) {
                if (conf[5 + 4 * i].at(j) == '1') {
                    (*fe->LID)[conf[5 + 4 * i].length() - j - 1] = true;
                } else {
                    (*fe->LID)[conf[5 + 4 * i].length() - j - 1] = false;
                }
            }
            forwarding_table.push_back(fe);
            if (port != 0) {
                click_chatter("Forwarder: Added forwarding entry: port %d - source MAC: %s - destination MAC: %s - LID: %s", fe->port, fe->src->unparse().c_str(), fe->dst->unparse().c_str(), fe->LID->to_string().c_str());
            } else {
                click_chatter("Forwarder: Added forwarding entry for the internal LINK ID: %s", fe->LID->to_string().c_str());
            }
        }
    } else {
        cp_integer(conf[1], &number_of_links);
        click_chatter("Forwarder: Number of Links: %d", number_of_links);
        for (int i = 0; i < number_of_links; i++) {
            cp_integer(conf[2 + 4 * i], &port);
            IPAddress * src_ip = new IPAddress();
            IPAddress * dst_ip = new IPAddress();
            cp_ip_address(conf[3 + 4 * i], src_ip, this);
            cp_ip_address(conf[4 + 4 * i], dst_ip, this);
            struct forwarding_entry *fe = (struct forwarding_entry *) CLICK_LALLOC(sizeof (struct forwarding_entry));
            fe->src_ip = src_ip;
            fe->dst_ip = dst_ip;
            fe->port = port;
            fe->LID = new BABitvector(FID_LEN * 8);
            for (int j = 0; j < conf[5 + 4 * i].length(); j++) {
                if (conf[5 + 4 * i].at(j) == '1') {
                    (*fe->LID)[conf[5 + 4 * i].length() - j - 1] = true;
                } else {
                    (*fe->LID)[conf[5 + 4 * i].length() - j - 1] = false;
                }
            }
            forwarding_table.push_back(fe);
            click_chatter("Forwarder: Added forwarding entry: port %d - source IP: %s - destination IP: %s - LID: %s", fe->port, fe->src_ip->unparse().c_str(), fe->dst_ip->unparse().c_str(), fe->LID->to_string().c_str());
        }
    }
    click_chatter("*********************************************************************************************************************************");
    //click_chatter("Forwarder: Configured!");
    return 0;
}

int Forwarder::initialize(ErrorHandler *errh) {
    //click_chatter("Forwarder: Initialized!");
    return 0;
}

void Forwarder::cleanup(CleanupStage stage) {
    int fw_size;
    if (stage >= CLEANUP_CONFIGURED) {
        fw_size = forwarding_table.size();
        for (int i = 0; i < fw_size; i++) {
            struct forwarding_entry *fe = forwarding_table.at(i);
            if (gc->use_mac == true) {
                click_chatter("deleting  forwarding entry: port %d - srcMAC: %s -dstMAC: %s - LID: %s", fe->port, fe->src->unparse().c_str(), fe->dst->unparse().c_str(), fe->LID->to_string().c_str());
                delete fe->src;
                delete fe->dst;
            } else {
                click_chatter("deleting  forwarding entry: port %d - srcIP: %s - dstIP: %s - LID: %s", fe->port, fe->src_ip->unparse().c_str(), fe->dst_ip->unparse().c_str(), fe->LID->to_string().c_str());
                delete fe->src_ip;
                delete fe->dst_ip;
            }
            delete fe->LID;
            CLICK_LFREE(fe, sizeof (struct forwarding_entry));
        }
    }
    //click_chatter("Forwarder: Cleaned Up!");
}

void Forwarder::push(int in_port, Packet *p) {
    WritablePacket *newPacket;
    WritablePacket *payload = NULL;
    struct forwarding_entry *fe;
    Vector<struct forwarding_entry *> out_links;
    BABitvector FID(FID_LEN * 8);
    BABitvector andVector(FID_LEN * 8);
    Vector<struct forwarding_entry *>::iterator out_links_it;
    int counter = 1;
    bool pushLocally = false;
    click_ip *ip;
    click_udp *udp;

    if (in_port == 0) {
        memcpy(FID._data, p->data(), FID_LEN);
        /*Check all entries in my forwarding table and forward appropriately*/
        for (int i = 0; i < forwarding_table.size(); i++) {
            fe = (struct forwarding_entry *) forwarding_table[i];
            andVector = (FID)&(*fe->LID);
            if (andVector == (*fe->LID)) {
                out_links.push_back(fe);
            }
        }
        if (out_links.size() == 0) {
            /*I can get here when an app or a click element did publish_data with a specific FID - Note that I never check if I can push back the packet above if it matches my iLID - the upper elements should check before pushing*/
            //click_chatter("killing packet - Forwarder");
            p->kill();
        }
        for (out_links_it = out_links.begin(); out_links_it != out_links.end(); out_links_it++) {
            if (counter == out_links.size()) {
                payload = p->uniqueify();
            } else {
                payload = p->clone()->uniqueify();
            }
            fe = *out_links_it;
            if (gc->use_mac) {
                //click_chatter("FROM HOST TO NETWORK VIA: %s", fe->LID->to_string().c_str());
                newPacket = payload->push_mac_header(14);
                /*prepare the mac header*/
                /*destination MAC*/
                memcpy(newPacket->data(), fe->dst->data(), MAC_LEN);
                /*source MAC*/
                memcpy(newPacket->data() + MAC_LEN, fe->src->data(), MAC_LEN);
                /*protocol type 0x080a*/
                memcpy(newPacket->data() + MAC_LEN + MAC_LEN, &proto_type, 2);
                /*push the packet to the appropriate ToDevice Element*/
                output(fe->port).push(newPacket);
            } else {
                newPacket = payload->push(sizeof (click_udp) + sizeof (click_ip));
                ip = reinterpret_cast<click_ip *> (newPacket->data());
                udp = reinterpret_cast<click_udp *> (ip + 1);
                // set up IP header
                ip->ip_v = 4;
                ip->ip_hl = sizeof (click_ip) >> 2;
                ip->ip_len = htons(newPacket->length());
                ip->ip_id = htons(_id.fetch_and_add(1));
                ip->ip_p = IP_PROTO_UDP;
                //click_chatter("setting source ip %s", fe->src_ip->unparse().c_str());
                //click_chatter("setting destination ip %s", fe->dst_ip->unparse().c_str());
                ip->ip_src = fe->src_ip->in_addr();
                ip->ip_dst = fe->dst_ip->in_addr();
                ip->ip_tos = 0;
                ip->ip_off = 0;
                ip->ip_ttl = 250;
                ip->ip_sum = 0;
                ip->ip_sum = click_in_cksum((unsigned char *) ip, sizeof (click_ip));
                newPacket->set_ip_header(ip, sizeof (click_ip));
                // set up UDP header
                udp->uh_sport = htons(7777);
                udp->uh_dport = htons(8888);
                uint16_t len = newPacket->length() - sizeof (click_ip);
                udp->uh_ulen = htons(len);
                udp->uh_sum = 0;
                unsigned csum = click_in_cksum((unsigned char *) udp, len);
                udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
                output(fe->port).push(newPacket);
                //click_chatter("sent packet via raw ip socket");
            }
            counter++;
        }
    } else if (in_port == 1) {
        //click_chatter("received packet via raw ip socket");
        //click_chatter("received a packet from the NETWORK");
        //click_chatter("/**a packet has been pushed by the underlying network.**/");
        /**a packet has been pushed by the underlying network.**/
        /*check if it needs to be forwarded*/
        if (gc->use_mac) {
            memcpy(FID._data, p->data() + 14, FID_LEN);
        } else {
            memcpy(FID._data, p->data() + 28, FID_LEN);
        }
        /*Check all entries in my forwarding table and forward appropriately*/
        for (int i = 0; i < forwarding_table.size(); i++) {
            fe = (struct forwarding_entry *) forwarding_table[i];
            andVector = (FID)&(*fe->LID);
            if (andVector == (*fe->LID)) {
                //click_chatter(" notify %d\n",fe->port);
                out_links.push_back(fe);
            }
        }
        /*check if the packet must be pushed locally*/
        andVector = (FID)&(*gc->iLID);
        if (andVector == (*gc->iLID)) {
            pushLocally = true;
        }
        for (out_links_it = out_links.begin(); out_links_it != out_links.end(); out_links_it++) {
            if ((counter == out_links.size()) && (pushLocally == false)) {
                payload = p->uniqueify();
            } else {
                payload = p->clone()->uniqueify();
            }
            fe = *out_links_it;
            //click_chatter("FROM NETWORK TO NETWORK VIA: %s", fe->LID->to_string().c_str());
            if (gc->use_mac) {
                /*prepare the mac header*/
                /*destination MAC*/
                memcpy(payload->data(), fe->dst->data(), MAC_LEN);
                /*source MAC*/
                memcpy(payload->data() + MAC_LEN, fe->src->data(), MAC_LEN);
                /*push the packet to the appropriate ToDevice Element*/
                //click_chatter("FW: payload->length(): %d, payload->headroom(): %d", payload->length(), payload->headroom());
                output(fe->port).push(payload);
                //click_chatter("2)forwarding packet with FID: %s from MAC Address %s to MAC address %s via output port %d", FID->to_string().c_str(), fe->src->unparse().c_str(), fe->dst->unparse().c_str(), fe->port);
            } else {
                click_ip *ip = reinterpret_cast<click_ip *> (payload->data());
                //click_chatter("setting source ip %s", fe->src_ip->unparse().c_str());
                //click_chatter("setting destination ip %s", fe->dst_ip->unparse().c_str());
                ip->ip_src = fe->src_ip->in_addr();
                ip->ip_dst = fe->dst_ip->in_addr();
                output(fe->port).push(payload);
            }
            counter++;
        }

        if (pushLocally) {
            if (gc->use_mac) {
                p->pull(14 + FID_LEN);
                output(0).push(p);
            } else {
                p->pull(20 + 8 + FID_LEN);
                output(0).push(p);
            }
        }

        if ((out_links.size() == 0) && (!pushLocally)) {
            p->kill();
        }
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(Forwarder)
