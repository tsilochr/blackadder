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


#include "network.hpp"

Domain::Domain() {
    TM_node = NULL;
    RV_node = NULL;
    number_of_nodes = 0;
    number_of_connections = 0;
}

void Domain::printDomainData() {
    int no_nodes;
    no_nodes = network_nodes.size();
    cout << "RV NODE: " << RV_node->label << endl;
    cout << "TM NODE: " << TM_node->label << endl;
    for (int i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        cout << "****************************" << endl;
        cout << "Node " << nn->label << endl;
        cout << "testbed_ip " << nn->testbed_ip << endl;
        cout << "iLid " << nn->iLid.to_string() << endl;
        cout << "isRV " << nn->isRV << endl;
        cout << "isTM " << nn->isTM << endl;
        cout << "FID_to_RV " << nn->FID_to_RV.to_string() << endl;
        cout << "FID_to_TM " << nn->FID_to_TM.to_string() << endl;
        cout << "****************************" << endl;
        for (int j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            cout << "*****src_label " << nc->src_label << endl;
            cout << "*****dst_label " << nc->dst_label << endl;
            cout << "*****src_if " << nc->src_if << endl;
            cout << "*****dst_if " << nc->dst_if << endl;
            cout << "*****src_ip " << nc->src_ip << endl;
            cout << "*****dst_ip " << nc->dst_ip << endl;
            cout << "*****src_mac " << nc->src_mac << endl;
            cout << "*****dst_mac " << nc->dst_mac << endl;
            cout << "*****LID " << nc->LID.to_string() << endl;
        }
    }
}

bool exists(vector<Bitvector> &LIDs, Bitvector &LID) {
    for (int i = 0; i < LIDs.size(); i++) {
        if (LIDs[i] == LID) {
            //cout << "duplicate LID" << endl;
            return true;
        }
    }
    return false;
}

NetworkNode *Domain::findNode(string label) {
    for (int i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        if (nn->label.compare(label) == 0) {
            return nn;
        }
    }
    return NULL;
}

void Domain::calculateLID(vector<Bitvector> &LIDs, int index) {
    int bit_position;
    int number_of_bits = (index / (fid_len * 8)) + 1;
    Bitvector LID;
    do {
        LID = Bitvector(fid_len * 8);
        for (int i = 0; i < number_of_bits; i++) {
            /*assign a bit in a random position*/
            bit_position = rand() % (fid_len * 8);
            LID[bit_position] = true;
        }
    } while (exists(LIDs, LID));
    LIDs[index] = LID;
}

void Domain::assignLIDs() {
    int LIDCounter = 0;
    srand(time(NULL));
    /*first calculated how many LIDs should I calculate*/
    int totalLIDs = number_of_nodes/*the iLIDs*/ + number_of_connections;
    vector<Bitvector> LIDs(totalLIDs);
    for (int i = 0; i < totalLIDs; i++) {
        calculateLID(LIDs, i);
    }
    for (int i = 0; i < totalLIDs; i++) {
        cout << "LID " << i << " : " << LIDs.at(i).to_string() << endl;
    }
    for (int i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        nn->iLid = LIDs[LIDCounter];
        LIDCounter++;
        for (int j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            nc->LID = LIDs[LIDCounter];
            LIDCounter++;
        }
    }
}

string Domain::getTestbedIPFromLabel(string label) {
    for (int i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        if (nn->label.compare(label) == 0) {
            return nn->testbed_ip;
        }
    }
}

void Domain::discoverMacAddresses() {
    string testbed_ip;
    string line;
    string mac_addr;
    FILE *fp_command;
    char response[1035];
    string command;
    for (int i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        for (int j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            if (overlay_mode.compare("mac") == 0) {
                /*get source mac address*/
                if (nc->src_mac.length() == 0) {
                    testbed_ip = getTestbedIPFromLabel(nc->src_label);
                    if (sudo) {
                        command = "ssh " + user + "@" + testbed_ip + " sudo ifconfig " + nc->src_if + " | grep HWaddr";
                    } else {
                        command = "ssh " + user + "@" + testbed_ip + " ifconfig " + nc->src_if + " | grep HWaddr";
                    }
                    //cout << command << endl;
                    fp_command = popen(command.c_str(), "r");
                    if (fp_command == NULL) {
                        printf("Failed to run command\n");
                    }
                    /* Read the output a line at a time - output it. */
                    fgets(response, sizeof (response) - 1, fp_command);
                    line = string(response);
                    mac_addr = line.substr(line.length() - 20, 17);
                    cout << mac_addr << endl;
                    nc->src_mac = mac_addr;
                    /* close */
                    pclose(fp_command);
                } else {
                    cout << "I already know the source mac address for this connection...it was hardcoded in the configuration file" << endl;
                }
                /*get destination mac address*/
                if (nc->dst_mac.length() == 0) {
                    testbed_ip = getTestbedIPFromLabel(nc->dst_label);
                    if (sudo) {
                        command = "ssh " + user + "@" + testbed_ip + " sudo ifconfig " + nc->src_if + " | grep HWaddr";
                    } else {
                        command = "ssh " + user + "@" + testbed_ip + " ifconfig " + nc->src_if + " | grep HWaddr";
                    }
                    //cout << command << endl;
                    fp_command = popen(command.c_str(), "r");
                    if (fp_command == NULL) {
                        printf("Failed to run command\n");
                    }
                    /* Read the output a line at a time - output it. */
                    fgets(response, sizeof (response) - 1, fp_command);
                    line = string(response);
                    mac_addr = line.substr(line.length() - 20, 17);
                    cout << mac_addr << endl;
                    nc->dst_mac = mac_addr;
                    /* close */
                    pclose(fp_command);
                } else {
                    cout << "I already know the destination mac address for this connection...it was hardcoded in the configuration file" << endl;
                }
            } else {
                //cout << "no need to discover mac addresses - it's a connection over raw IP sockets" << endl;
            }
        }
    }
}

int findOffset(vector<string> &unique, string &str) {
    for (int i = 0; i < unique.size(); i++) {
        if (unique[i].compare(str) == 0) {
            return i;
        }
    }
    return -1;
}

void Domain::writeClickFiles() {
    ofstream click_conf;

    for (int i = 0; i < network_nodes.size(); i++) {
        vector<string> unique_ifaces;
        vector<string> unique_srcips;
        NetworkNode *nn = network_nodes[i];
        click_conf.open((write_conf + nn->label + ".conf").c_str());
        click_conf << "require(blackadder);" << endl << endl;
        /*Blackadder Elements First*/
        click_conf << "globalconf::GlobalConf(MODE " << overlay_mode << ", NODEID " << nn->label << "," << endl;
        click_conf << "DEFAULTRV " << nn->FID_to_RV.to_string() << "," << endl;
        click_conf << "TMFID     " << nn->FID_to_TM.to_string() << "," << endl;
        click_conf << "iLID      " << nn->iLid.to_string() << ");" << endl << endl;

        click_conf << "localRV::LocalRV(globalconf," << nn->connections.size() /*number of neighbours*/ << "," << endl;
        for (int j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            NetworkNode *dest_nn = findNode(nc->dst_label);
            if (j == nn->connections.size() - 1) {
                click_conf << /*a neighbour*/dest_nn->label << "," << /*LID to it*/dest_nn->iLid.to_string() << "," << /*iLID of it*/ nc->LID.to_string() << ");" << endl << endl;
            } else {
                click_conf << /*a neighbour*/dest_nn->label << "," << /*LID to it*/dest_nn->iLid.to_string() << "," << /*iLID of it*/ nc->LID.to_string() << "," << endl;
            }
        }
        click_conf << "netlink::Netlink();" << endl << "tonetlink::ToNetlink(netlink);" << endl << "fromnetlink::FromNetlink(netlink);" << endl << endl;
        click_conf << "proxy::LocalProxy(globalconf);" << endl << endl;

        click_conf << "fw::Forwarder(globalconf," << nn->connections.size() << "," << endl;
        for (int j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            int offset;
            if (overlay_mode.compare("mac") == 0) {
                if ((offset = findOffset(unique_ifaces, nc->src_if)) == -1) {
                    unique_ifaces.push_back(nc->src_if);
                    if (j == nn->connections.size() - 1) {
                        click_conf << unique_ifaces.size() << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string() << ");" << endl << endl;
                    } else {
                        click_conf << unique_ifaces.size() << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string() << "," << endl;
                    }
                } else {
                    if (j == nn->connections.size() - 1) {
                        click_conf << offset + 1 << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string() << ");" << endl << endl;
                    } else {
                        click_conf << offset + 1 << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string() << "," << endl;
                    }
                }
            } else {
                if ((offset = findOffset(unique_srcips, nc->src_ip)) == -1) {
                    unique_srcips.push_back(nc->src_ip);
                    //cout << "PUSHING BACK " << nc->src_ip << endl;
                    //cout << unique_srcips.size() << endl;
                    if (j == nn->connections.size() - 1) {
                        click_conf << unique_srcips.size() << "," << nc->src_ip << "," << nc->dst_ip << "," << nc->LID.to_string() << ");" << endl << endl;
                    } else {
                        click_conf << unique_srcips.size() << "," << nc->src_ip << "," << nc->dst_ip << "," << nc->LID.to_string() << "," << endl;
                    }
                } else {
                    if (j == nn->connections.size() - 1) {
                        click_conf << offset + 1 << "," << nc->src_ip << "," << nc->dst_ip << "," << nc->LID.to_string() << ");" << endl << endl;
                    } else {
                        click_conf << offset + 1 << "," << nc->src_ip << "," << nc->dst_ip << "," << nc->LID.to_string() << "," << endl;
                    }
                }
            }
        }
        if (overlay_mode.compare("mac") == 0) {
            for (int j = 0; j < unique_ifaces.size(); j++) {
                click_conf << "tsf" << j << "::ThreadSafeQueue(1000);" << endl;
                if (nn->running_mode.compare("user") == 0) {
                    click_conf << "fromdev" << j << "::FromDevice(" << unique_ifaces[j] << ");" << endl << "todev" << j << "::ToDevice(" << unique_ifaces[j] << ");" << endl;
                } else {
                    click_conf << "fromdev" << j << "::FromDevice(" << unique_ifaces[j] << ", BURST 8);" << endl << "todev" << j << "::ToDevice(" << unique_ifaces[j] << ", BURST 8);" << endl;
                }
            }
            /*Necessary Click Elements*/
            if (nn->running_mode.compare("user") == 0) {
                click_conf << "classifier::Classifier(12/080a);" << endl;
            } else {
                click_conf << "classifier::Classifier(12/080a, -);" << endl;
                click_conf << "tohost::ToHost();" << endl;
            }
        } else {
            /*raw sockets here*/
            click_conf << "tsf" << "::ThreadSafeQueue(1000);" << endl;
            if (nn->running_mode.compare("user") == 0) {
                click_conf << "rawsocket" << "::RawSocket(UDP, 7777)" << endl;
                click_conf << "classifier::IPClassifier(dst udp port 8888 and src udp port 7777)" << endl;
            } else {
                cerr << "Something is wrong...I should not build click config using raw sockets for node " << nn->label << "that will run in kernel space" << endl;
            }
        }

        /*Now link all the elements appropriately*/
        click_conf << endl << endl << "proxy[0]->tonetlink;" << endl << "fromnetlink->[0]proxy;" << endl << "localRV[0]->[1]proxy[1]->[0]localRV;" << endl << "proxy[2]-> [0]fw[0] -> [2]proxy;" << endl;
        if (overlay_mode.compare("mac") == 0) {
            for (int j = 0; j < unique_ifaces.size(); j++) {
                click_conf << "fw[" << (j + 1) << "]->tsf" << j << "->todev" << j << ";" << endl;
                click_conf << "fromdev" << j << "->classifier[0]->[" << (j + 1) << "]fw;" << endl;
            }
            if (nn->running_mode.compare("kernel") == 0) {
                click_conf << "classifier[1]->tohost;" << endl;
            }
        } else {
            /*raw sockets here*/
            click_conf << "fw[1] ->  tsf -> rawsocket -> classifier -> [1]fw" << endl;
        }
        click_conf.close();
    }
}

void Domain::scpTMConfiguration(string TM_conf) {
    FILE *scp_command;
    string command;
    command = "scp " + write_conf + TM_conf  + " " + user + "@" + TM_node->testbed_ip + ":" + write_conf;
    cout << command << endl;
    scp_command = popen(command.c_str(), "r");
    /* close */
    pclose(scp_command);
}

void Domain::scpClickFiles() {
    FILE *scp_command;
    char response[1035];
    string command;
    for (int i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        command = "scp " + write_conf + nn->label + ".conf" + " " + user + "@" + nn->testbed_ip + ":" + write_conf;
        cout << command << endl;
        scp_command = popen(command.c_str(), "r");
        if (scp_command == NULL) {
            cerr << "Failed to scp click file to node " << nn->label << endl;
        }
        /* close */
        pclose(scp_command);
    }
}

void Domain::startClick() {
    FILE *ssh_command;
    string command;
    for (int i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];

        /*kill click first both from kernel and user space*/
        if (sudo) {
            command = "ssh " + user + "@" + nn->testbed_ip + " \"sudo pkill -9 click\"";
        } else {
            command = "ssh " + user + "@" + nn->testbed_ip + " \"pkill -9 click\"";
        }
        cout << command << endl;
        ssh_command = popen(command.c_str(), "r");
        if (ssh_command == NULL) {
            cerr << "Failed to stop click at node " << nn->label << endl;
        }
        pclose(ssh_command);


        if (sudo) {
            command = "ssh " + user + "@" + nn->testbed_ip + " \"sudo " + click_home + "sbin/click-uninstall\" ";
        } else {
            command = "ssh " + user + "@" + nn->testbed_ip + " \"" + click_home + "sbin/click-uninstall \"";
        }
        cout << command << endl;
        ssh_command = popen(command.c_str(), "r");
        if (ssh_command == NULL) {
            cerr << "Failed to stop click at node " << nn->label << endl;
        }
        pclose(ssh_command);


        /*now start click*/
        if (nn->running_mode.compare("user") == 0) {
            if (sudo) {
                command = "ssh " + user + "@" + nn->testbed_ip + " \"sudo " + click_home + "bin/click " + write_conf + nn->label + ".conf > /dev/null 2>&1 &\"";
            } else {
                command = "ssh " + user + "@" + nn->testbed_ip + " \"" + click_home + "bin/click " + write_conf + nn->label + ".conf > /dev/null 2>&1 &\"";
            }
            cout << command << endl;
            ssh_command = popen(command.c_str(), "r");
            if (ssh_command == NULL) {
                cerr << "Failed to start click at node " << nn->label << endl;
            }
            pclose(ssh_command);
        } else {
            if (sudo) {
                command = "ssh " + user + "@" + nn->testbed_ip + " \"sudo " + click_home + "sbin/click-install " + write_conf + nn->label + ".conf > /dev/null 2>&1 &\"";
            } else {
                command = "ssh " + user + "@" + nn->testbed_ip + " \"" + click_home + "sbin/click-install " + write_conf + nn->label + ".conf > /dev/null 2>&1 &\"";
            }
            cout << command << endl;
            ssh_command = popen(command.c_str(), "r");
            if (ssh_command == NULL) {
                cerr << "Failed to start click at node " << nn->label << endl;
            }
            pclose(ssh_command);
        }
    }
}

void Domain::startTM() {

}