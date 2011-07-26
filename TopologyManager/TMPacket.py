# -*- coding: utf-8 -*-
# TmPacket.py contains TmPacket Header Request and Response classes
# Author: Dimitris Syrivelis
# ITI CERTH for Pursuit project. This source is under BlackAdder license 

from impacket import ImpactPacket
from array import *
import struct
import socket
import string
import sys
from binascii import hexlify

MATCH_PUB_SUBS = 105
SCOPE_PUBLISHED = 102

class TmRequest(ImpactPacket.Header):
    """ This packet is the format for TM Requests  """
    protocol = 1
    dataoffset = 2
    def __init__(self, totalsize, IDlen, aBuffer = None):
        ImpactPacket.Header.__init__(self, totalsize)
        self.IDlen = IDlen
        self.psize = totalsize
        if(aBuffer):
	    self.load_header(aBuffer)
	    
    def setRequestType(self, num):
	self.set_byte(0, num)
	
    def getRequestType(self):
	return self.get_byte(0)
       
    def getNofSubscribers(self):
        offset = self.skipPublishers()
	return self.get_byte(offset)
	
    def setNofPublishers(self, num):
	self.set_byte(1, num)
	
    def getNofPublishers(self):
	return self.get_byte(1)
	
    def getNofPathIds(self):
	offset = self.skipSubscribersPublishers()
	return self.get_byte(offset)

    def getNofPathIdsSpecial(self):
	offset = self.skipPublishers()
	return self.get_byte(offset) 

    def skipNextEntry(self,offset):
	return (offset + (self.get_byte(offset)*self.IDlen) + 1)

    def skipPublishers(self):
	offset = self.dataoffset + (self.IDlen * self.getNofPublishers())  
	return offset

    def skipSubscribersPublishers(self):
	offset = self.dataoffset + 1 + (self.IDlen * (self.getNofPublishers() + self.getNofSubscribers())) 
	return offset

    def getPublisher(self, offset):
	start = self.dataoffset
	for i in range(0, offset):
		start = start + self.IDlen
	return self.get_bytes()[start:(start+self.IDlen)].tostring() 

    def getPathId(self, offset):
	start = self.skipSubscribersPublishers() + 1
	for i in range(0, offset):
		start = self.skipNextEntry(start)
	length = self.get_byte(start) * self.IDlen
	start = start + 1
	return self.get_bytes()[start:(start+length)]

    def getPathIdSpecial(self, offset):
        start = self.skipPublishers() + 1
        for i in range(0, offset):
                start = self.skipNextEntry(start)
        length = self.get_byte(start) * self.IDlen
        start = start + 1
        return self.get_bytes()[start:(start+length)]

	
    def getSubscriber(self, offset):
	start = self.skipPublishers() + 1
	for i in range(0, offset):
		start = start + self.IDlen
	return self.get_bytes()[start:(start+self.IDlen)].tostring() 

    def getNofPathIdslen(self):
	start = self.skipSubscribersPublishers()
        length = 0
        nofpathids = self.getNofPathIds()
	for i in range(0, nofpathids):
		length = self.get_byte(start) * self.IDlen
		start = self.skipNextEntry(start)
	return length

    def get_header_size(self):
	    return (self.psize)


class TmResponse(ImpactPacket.Header):
    ''' This packet carries the response to the BlackAdder '''
    protocol = 1
    dataoffset = 2
    psize = 2
    bvsize = 0
    def __init__(self, bvlen, IDlen, aBuffer = None):
        ImpactPacket.Header.__init__(self, 2)
	self.bvsize = bvlen
	self.IDlen = IDlen
        if(aBuffer):
	    self.load_header(aBuffer)
	    self.bvsize = self.getSizeofBitVector()
	       
    def setNofPathIds(self, num):
	self.set_byte(1, num)
	
    def getnumberofPathID(self):
	return self.get_byte(1)
   
    def setType(self, num):
	self.set_byte(0, num)
	
    def getType(self):
	return self.get_byte(0,num)

    def skipNextEntry(self,offset):
	return (offset + (self.get_byte(offset)*self.IDlen) + 1)

    def setPathId(self,offset, pid, length):
	start = self.dataoffset
	for i in range(0, offset):
		start = self.skipNextEntry(start)
	self.set_byte(start, (length/self.IDlen))
	start = start + 1
	self.psize = self.psize + length + 1
	# incrementally increase the buffer size to fit the pathid
        self.get_bytes().fromstring('\0' * length)
	self.get_bytes()[start:(start+length)] = pid

    def skipPathIds(self):
	offset = self.dataoffset
	for i in range(0, self.getnumberofPathID()):
		 offset = self.skipNextEntry(offset)
	return offset

    def setFIDEntry(self, fid):
	'''Adds an entry to the packet for a publisher+'''
	start = self.skipPathIds()
	#add the bitVector as a 32-bit integer to properly decode it at the blackadder engine, 4-byte serialized integer values are only allowed. 
	#position of integers on the array also matters 
	bitvectoruint32t = self.bvsize / 4
	for i in range(0, bitvectoruint32t):
		datachunk = 0x00000000		
		currentCycle = 32*(bitvectoruint32t - i) - 1
		for j in range(0, 32):
			if(fid[currentCycle-j] == 1):
				datachunk = datachunk | (1 << j)
		self.psize = self.psize+4
		self.set_long(start, datachunk, order='=')
		start = start + 4
	   
    def get_header_size(self):
	    return self.psize


