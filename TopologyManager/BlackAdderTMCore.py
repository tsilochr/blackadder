# -*- coding: utf-8 -*-
# BlackAdderTMCore.py  contains TM core packet scheduler
# Author: Dimitris Syrivelis
# ITI CERTH for Pursuit project. This source is under BlackAdder license 

from impacket import ImpactPacket
from threading import Thread
import array
import struct
import socket
import string
import sys
import Queue
from binascii import hexlify
from config import Config


class TMPacketSchedulerModule(Thread):

    def __init__(self):
	self.PacketInQueue = Queue.Queue()
	Thread.__init__(self)

    def kill(self):
	Thread.join(self)	
	    	    
    def main_loop(self):
	    '''Main Loop hosts the generic main loop - it must be overriden'''
	    raise RuntimeError("Method %s.main_loop must be overriden." % self.__class__)
	
    def GetPacket(self):
	return self.PacketInQueue.get()
    	
    def IngressPut(self, pckt):
	self.PacketInQueue.put(pckt, False)
    	
    def run(self):
	self.main_loop()
