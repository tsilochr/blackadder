/* 
 * File:   main.cpp
 * Author: George Parisis
 *
 * Created on July 22, 2011, 11:04 AM
 */

#include <iostream>
#include <iomanip>
#include <cstdlib>

#include "network.hpp"
#include "graph_representation.hpp"
#include "parser.hpp"

using namespace std;

int main(int argc, char **argv) {
    int ret;
    if (argc != 2) {
        std::cout << "Usage: deploy <filename>" << endl;
        return (EXIT_FAILURE);
    }
    Domain dm = Domain();
    Parser parser(argv[1], &dm);
    GraphRepresentation graph = GraphRepresentation(&dm);
    ret = parser.buildNetworkDomain();
    if (ret < 0) {
        cout << "Something went wrong" << endl;
        return EXIT_FAILURE;
    }

    dm.assignLIDs();
    graph.buildIGraphTopology();
    graph.calculateRVFIDs();
    graph.calculateTMFIDs();
    dm.discoverMacAddresses();
    dm.writeClickFiles();
    dm.scpClickFiles();
    dm.startClick();
    dm.startTM();

    /*set some graph attributes for the topology manager*/
    igraph_cattribute_GAN_set(&graph.igraph, "FID_LEN", dm.fid_len);
    igraph_cattribute_GAS_set(&graph.igraph, "TM", dm.TM_node->label.c_str());
    igraph_cattribute_GAS_set(&graph.igraph, "TM_MODE", dm.TM_node->running_mode.c_str());
    FILE * outstream_graphml = fopen(string(dm.write_conf + "topology.graphml").c_str(), "w");
    igraph_write_graph_graphml(&graph.igraph, outstream_graphml);
    fclose(outstream_graphml);
    dm.scpTMConfiguration("topology.graphml");

    //dm.printDomainData();
}
