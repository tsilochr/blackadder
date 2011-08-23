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