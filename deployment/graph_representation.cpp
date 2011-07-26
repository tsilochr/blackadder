/* 
 * File:   graph.cpp
 * Author: George Parisis
 * 
 * Created on July 22, 2011, 11:11 AM
 */
#include <vector>

#include "graph_representation.hpp"

GraphRepresentation::GraphRepresentation(Domain *_dm) {
    dm = _dm;
}

int GraphRepresentation::buildIGraphTopology() {
    int source_vertex_id, destination_vertex_id;
    igraph_i_set_attribute_table(&igraph_cattribute_table);
    igraph_empty(&igraph, 0, true);
    //cout << "iGraph: number of nodes: " << dm->number_of_nodes << endl;
    //cout << "iGraph: number number_of_connections nodes: " << dm->number_of_connections << endl;
    for (int i = 0; i < dm->network_nodes.size(); i++) {
        NetworkNode *nn = dm->network_nodes[i];
        for (int j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            map <string, int>::iterator index_it;
            /*find that node - if exists*/
            index_it = reverse_node_index.find(nc->src_label);
            /*check if the source node exists in the reverse index*/
            if (index_it == reverse_node_index.end()) {
                /*it does not exist...add it*/
                source_vertex_id = igraph_vcount(&igraph);
                igraph_add_vertices(&igraph, 1, 0);
                reverse_node_index.insert(pair<string, int>(nc->src_label, source_vertex_id));
                igraph_cattribute_VAS_set(&igraph, "NODEID", source_vertex_id, nc->src_label.c_str());
                //cout << "added node " << nc->src_label << " in the igraph" << endl;
            } else {
                source_vertex_id = (*index_it).second;
            }
            index_it = reverse_node_index.find(nc->dst_label);
            /*check if the destination node exists in the reverse index*/
            if (index_it == reverse_node_index.end()) {
                /*it does not exist...add it*/
                destination_vertex_id = igraph_vcount(&igraph);
                igraph_add_vertices(&igraph, 1, 0);
                reverse_node_index.insert(pair<string, int>(nc->dst_label, destination_vertex_id));
                igraph_cattribute_VAS_set(&igraph, "NODEID", destination_vertex_id, nc->dst_label.c_str());
                //cout << "added node " << nc->dst_label << " in the igraph" << endl;
            } else {
                destination_vertex_id = (*index_it).second;
            }
            /*add an edge in the graph*/
            igraph_add_edge(&igraph, source_vertex_id, destination_vertex_id);
            igraph_cattribute_EAS_set(&igraph, "LID", igraph_ecount(&igraph) - 1, nc->LID.to_string().c_str());
            reverse_edge_index.insert(pair<string, int>(nc->LID.to_string(), igraph_ecount(&igraph) - 1));
        }
    }
    for (int i = 0; i < dm->network_nodes.size(); i++) {
        NetworkNode *nn = dm->network_nodes[i];
        igraph_cattribute_VAS_set(&igraph, "iLID", i, nn->iLid.to_string().c_str());
    }
}

Bitvector GraphRepresentation::calculateFID(string &source, string &destination) {
    int vertex_id;
    Bitvector result(dm->fid_len * 8);
    igraph_vs_t vs;
    igraph_vector_ptr_t res;
    igraph_vector_t to_vector;
    igraph_vector_t *temp_v;
    igraph_integer_t eid;

    /*find the vertex id in the reverse index*/
    int from = (*reverse_node_index.find(source)).second;
    igraph_vector_init(&to_vector, 1);
    VECTOR(to_vector)[0] = (*reverse_node_index.find(destination)).second;
    /*initialize the sequence*/
    igraph_vs_vector(&vs, &to_vector);
    /*initialize the vector that contains pointers*/
    igraph_vector_ptr_init(&res, 1);
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    temp_v = (igraph_vector_t *) malloc(sizeof (igraph_vector_t));
    VECTOR(res)[0] = temp_v;
    igraph_vector_init(temp_v, 1);
    /*run the shortest path algorithm from "from"*/
    igraph_get_shortest_paths(&igraph, &res, from, vs, IGRAPH_OUT);
    /*check the shortest path to each destination*/
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    //click_chatter("Shortest path from %s to %s", igraph_cattribute_VAS(&graph, "NODEID", from), igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[igraph_vector_size(temp_v) - 1]));

    /*now let's "or" the FIDs for each link in the shortest path*/
    for (int j = 0; j < igraph_vector_size(temp_v) - 1; j++) {
        igraph_get_eid(&igraph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true);
        //click_chatter("node %s -> node %s", igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[j]), igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[j + 1]));
        //click_chatter("link: %s", igraph_cattribute_EAS(&graph, "LID", eid));
        string LID(igraph_cattribute_EAS(&igraph, "LID", eid), dm->fid_len * 8);
        for (int k = 0; k < dm->fid_len * 8; k++) {
            if (LID[k] == '1') {
                (result)[ dm->fid_len * 8 - k - 1].operator |=(true);
            }
        }
        //click_chatter("FID of the shortest path: %s", result.to_string().c_str());
    }
    /*now for all destinations "or" the internal linkID*/
    vertex_id = (*reverse_node_index.find(destination)).second;
    string iLID(igraph_cattribute_VAS(&igraph, "iLID", vertex_id));
    //click_chatter("internal link for node %s: %s", igraph_cattribute_VAS(&graph, "NODEID", vertex_id), iLID.c_str());
    for (int k = 0; k < dm->fid_len * 8; k++) {
        if (iLID[k] == '1') {
            (result)[ dm->fid_len * 8 - k - 1].operator |=(true);
        }
    }

    igraph_vector_destroy((igraph_vector_t *) VECTOR(res)[0]);

    igraph_vector_destroy(&to_vector);
    igraph_vector_ptr_destroy_all(&res);
    igraph_vs_destroy(&vs);

    return result;
}

void GraphRepresentation::calculateRVFIDs() {
    string RVLabel = dm->RV_node->label;
    for (int i = 0; i < dm->network_nodes.size(); i++) {
        NetworkNode *nn = dm->network_nodes[i];
        nn->FID_to_RV = calculateFID(nn->label, RVLabel);
    }
}

void GraphRepresentation::calculateTMFIDs() {
    string TMLabel = dm->TM_node->label;
    for (int i = 0; i < dm->network_nodes.size(); i++) {
        NetworkNode *nn = dm->network_nodes[i];
        nn->FID_to_TM = calculateFID(nn->label, TMLabel);
    }
}