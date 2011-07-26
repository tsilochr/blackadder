#!/usr/bin/python
# -*- coding: utf-8 -*-
# main.py TmEngine entry point.
# Author: Dimitris Syrivelis
# ITI CERTH for Pursuit project. This source is under BlackAdder license 

import select
import socket
import time
import signal
import struct
import TMIgraph
import BlackAdderTMCore
import TMPacket
import BlackAdderConnection
from array import *
import TMConfigParser
import sys
import os
import math
import string
import traceback
import getopt
import BitVector
from config import Config
from impacket import ImpactPacket

global tigraph

def signal_handler(signal, frame):
    '''This is the signal handler for Ctrl-C, it does not work as expected...'''
    print 'Graceful Exit due to Ctrl-C!'
    bcon.disconnect()
    tigraph.kill()
    sys.exit(0)

#Welcome Message
print "====BlackAdder Topology Manager Version 0.1===="

#Execution begins here. Firstly register a handler for Ctrl-C
signal.signal(signal.SIGINT, signal_handler)

#TmIgraph module gets configuration maps, uses igraph library to handle packet requests and produces response packets to the outgoing Queue
tigraph = TMIgraph.TMIgraphModule(sys.argv[1])

# Open the connection to BlackAdder
bcon = BlackAdderConnection.Connection(tigraph.mode)

tigraph.setCon(bcon)

#Subscribe to the hardcoded TM scope
tmscopeid = array('B',[0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe])
tmprefixid = array('B', [])
bcon.publishAction(bcon.SUBSCRIBE_SCOPE, tmscopeid.tostring(), tmprefixid.tostring(), bcon.SUBSCRIBE_LOCALLY)

tigraph.start()
	
#ingress main loop. receives packets from BlackAdder Netlink interface and adds them to a processor queue. Currently only TMigraph is available 
while 1:
   pckt=bcon.recv()
   if(pckt[0] == bcon.PUBLISHED_DATA):
      data = pckt[3]
      tigraph.IngressPut(data)

