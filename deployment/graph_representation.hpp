/* 
 * File:   graph.hpp
 * Author: George Parisis
 *
 * Created on July 22, 2011, 11:11 AM
 */

#ifndef GRAPH_REPRESENTATION_HPP
#define	GRAPH_REPRESENTATION_HPP

#include <map>
#include <vector>
#include "bitvector.hpp"
#include <igraph/igraph.h>
#include "network.hpp"

using namespace std;

class GraphRepresentation {
public:
    GraphRepresentation(Domain *_dm);
    int buildIGraphTopology();
    Bitvector calculateFID(string &source, string &destination);
    void calculateRVFIDs();
    void calculateTMFIDs();
    igraph_t igraph;
    map <string, int> reverse_node_index;
    map <string, int> reverse_edge_index;
    
    Domain *dm;
};

#endif