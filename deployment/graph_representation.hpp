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