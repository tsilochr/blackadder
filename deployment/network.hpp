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



#ifndef NETWORK_HPP
#define	NETWORK_HPP


#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>

#include "bitvector.hpp"

using namespace std;

class NetworkConnection;
class NetworkNode;

class Domain {
public:
    Domain();
    /*Network related parameters*/
    vector<NetworkNode *> network_nodes;
    /*ONLY ONE RV AND TM ARE ALLOWED IN A DOMAIN*/
    NetworkNode *TM_node;
    NetworkNode *RV_node;
    unsigned int number_of_nodes;
    unsigned int number_of_connections;
    int ba_id_len;
    int fid_len;
    string click_home;
    string write_conf;
    string user;
    bool sudo;
    string overlay_mode; //mac or ip
    /****************************/
    void printDomainData();
    void assignLIDs();
    void calculateLID(vector<Bitvector> &LIDs, int index);
    void discoverMacAddresses();
    string getTestbedIPFromLabel(string label);
    void writeClickFiles();
    NetworkNode *findNode(string label);
    void scpClickFiles();
    void scpTMConfiguration(string );
    void startClick();
    void startTM();
};

class NetworkNode {
public:
    /***members****/
    string testbed_ip; //read from configuration file
    string label; //read from configuration file
    string running_mode; //user or kernel
    bool isRV; //read from configuration file
    bool isTM; //read from configuration file
    Bitvector iLid; //will be calculated
    Bitvector FID_to_RV; //will be calculated
    Bitvector FID_to_TM; //will be calculated
    vector<NetworkConnection *> connections;
};

class NetworkConnection {
public:
    /***members****/
    string src_label; //read from configuration file
    string dst_label; //read from configuration file

    string src_if; //read from configuration file /*e.g. tap0 or eth1*/
    string dst_if; //read from configuration file /*e.g. tap0 or eth1*/

    string src_ip; //read from configuration file /*an IP address - i will not resolve mac addresses in this case*/
    string dst_ip; //read from configuration file /*an IP address - i will not resolve mac addresses in this case*/

    string src_mac; //will be retrieved using ssh
    string dst_mac; //will be retrieved using ssh
    Bitvector LID; //will be calculated
};

#endif	/* NETWORK_HPP */

