/* 
 * File:   parser.hpp
 * Author: George Parisis
 *
 * Created on July 22, 2011, 12:02 PM
 */

#ifndef PARSER_HPP
#define	PARSER_HPP

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

#include "network.hpp"

using namespace std;
using namespace libconfig;

class Parser {
public:
    Parser(char *_file, Domain *_dm);
    Parser(const Parser& orig);
    virtual ~Parser();
    int parseConfiguration();
    int buildNetworkDomain();
    int getGlobalDomainParameters();
    int addNode(const Setting &node);
    int addConnection(const Setting &connection, NetworkNode *nn);
private:
    char *file;
    Config cfg;
    Domain *dm;
};

#endif	/* PARSER_HPP */