# -*- coding: utf-8 -*-
# TmIgraph.py Tm algorithm module that uses igraph lib
# Author: Dimitris Syrivelis, most of this code is a port from George Parisis' click tm module
# CERTH for FP7 Pursuit project. This source is under BlackAdder license 

from impacket import ImpactPacket
import TMPacket
import igraph
from array import *
import struct
import socket
import string
import time
import sys
import BlackAdderTMCore
import BitVector
import time
from binascii import hexlify

class TMIgraphModule(BlackAdderTMCore.TMPacketSchedulerModule):
    '''This Class implements the Igraph-based TM module'''
    labelmap = {}
    LIDmap = {}

    def __init__(self,filename):
	self.gh=igraph.Graph.Read_GraphML(filename)
	self.fid_len = int(self.gh["FID_LEN"])
	self.TMId = self.gh["TM"]
	self.mode = self.gh["TM_MODE"]
	print "FID LENGTH: %s" %self.fid_len
	print "I am node: %s" %self.TMId
	print "I am running in %s-space" %self.mode
	BlackAdderTMCore.TMPacketSchedulerModule.__init__(self)
	for i in range(0, self.gh.vcount()):
		self.labelmap[self.gh.vs[i]['NODEID']]=i
		ivec = BitVector.BitVector(size = (self.fid_len * 8))
		str_iLID = self.gh.vs[i]['iLID']
		#print "%s" %str_iLID
		for j in range(0, len(str_iLID)):
			if (str_iLID[j] == '0'):
				ivec[j] = 0
			else:
				ivec[j] = 1
		self.LIDmap[str_iLID]= ivec
		#print "%s" %ivec
	for i in range(0, self.gh.ecount()):
		vec = BitVector.BitVector(size = (self.fid_len * 8))
		str_LID = self.gh.es[i]['LID']
		#print "%s" %str_LID
		for j in range(0, len(str_LID)):
			if (str_LID[j] == '0'):
				vec[j] = 0
			else:
				vec[j] = 1
		self.LIDmap[str_LID]= vec
		#print "%s" %vec
	#for k, v in self.LIDmap.iteritems():       
	#	print "%s=%s" % (k, v)
	
		
    def setCon(self, con):
	self.bcon = con
    
    def retrieveVertexfromNodeId(self,nodeid):
	#print "retrieveVertexfromNodeId:  nodeid %s" %(nodeid)
	index = self.labelmap[nodeid]
	return index

    #make the 8 a parameter please
    def GetFIDfromPath(self, vertices):
	lr = len(vertices)-1
        if (lr == 0):
	   LID = BitVector.BitVector(size=(self.fid_len*8))
	   return LID
	LID = self.LIDmap[self.gh.es[self.gh.get_eid(vertices[0],vertices[1])]['LID']]
	#LID = self.gh.es[self.gh.get_eid(vertices[0],vertices[1])]['LID']
	for i in range (1, lr):
		LID = LID | self.LIDmap[self.gh.es[self.gh.get_eid(vertices[i],vertices[i+1])]['LID']]
		#LID = LID | self.gh.es[self.gh.get_eid(vertices[i],vertices[i+1])]['LID']
	return LID
  

    def calculateMultiFID(self, publishers, subscribers):
	'''calculate FIDs from many publishers to many subscribers'''
	result = {}
	for i in range(0, len(publishers)):
		result[publishers[i]] = BitVector.BitVector(size = (self.fid_len * 8))

	for i in range(0, len(subscribers)):
		minimumNumberOfHops = 999999
		bestPublisher = publishers[0]
		bestFID = BitVector.BitVector(size = (self.fid_len * 8))		
		for j in range(0, len(publishers)):
			resultTuple = self.calculateFID(publishers[j], subscribers[i])
			numberofHops = resultTuple['Hops']
			if (minimumNumberOfHops > numberofHops):
     	                	minimumNumberOfHops = numberofHops
                		bestPublisher = publishers[j]
                		bestFID = resultTuple['FID']
		#here actual best publisher has been determined
		print "TMINFO: For Subscriber %s the BestPublisher is %s and subsequent FID is %s" % (subscribers[i], bestPublisher, bestFID)
		result[bestPublisher] = result[bestPublisher] | bestFID  		
	return result
		

    def calculateFID(self, source, destination):
	'''This function returns a dict that holds FID and num-of-hops pairs'''
	print "TMINFO: <CalculateFID> The Source is %s and Destination %s" % (source , destination)
	pathstoalldestinations = self.gh.get_shortest_paths(self.retrieveVertexfromNodeId(source))
	for i in range(0, len(pathstoalldestinations)):
		if (i == self.retrieveVertexfromNodeId(destination)):
			FID =  self.GetFIDfromPath(pathstoalldestinations[i])
			#print "FID1 %s" %(FID)
			iLID = 	self.LIDmap[self.gh.vs[self.retrieveVertexfromNodeId(destination)]['iLID']]
			#print "iLID: %s" %iLID
			FID = FID | iLID
			print "TMINFO: <CalculateFID> FID result: %s, Number of Hops %s" %(FID, (len(pathstoalldestinations[i]) - 1))
			return { 'FID': FID , 'Hops': (len(pathstoalldestinations[i]) - 1) } 
	return
    

    def getFIDFromTMToPublisher(self, publisherid):
	return self.calculateFID(self.TMId, publisherid)['FID']

    def calculateResponseSize(self, numofPublishers, TotalPubLength, bvlen):
	'''Calculate TM Response Size'''
	return (6 + numofPublishers + TotalPubLength + (numofPublishers * bvlen))

    def HandleTMIGraphPacket(self, packet):
	'''This function handles TMIgraph packets'''
	print "\n===============================\n"
	inpckt = TMPacket.TmRequest(len(packet), 8, packet)
	#Holds Subscriber array
	subs = []
	#Holds Publisher array
	pubs = []
	Type = inpckt.getRequestType()
        if (Type == TMPacket.MATCH_PUB_SUBS):
	    for i in range(0, inpckt.getNofSubscribers()):
	        subs.append(inpckt.getSubscriber(i))
	     #Get Publisher array
	    for i in range(0, inpckt.getNofPublishers()):
	        pubs.append(inpckt.getPublisher(i))
	    #Calculate Fids for BestPublisher to subscribers
	    resultpubs = self.calculateMultiFID(pubs, subs)
            nullBitVector = BitVector.BitVector(size = (self.fid_len * 8))
            for i in range(0, len(pubs)):
	   	   #if a request for senders to publishers fid is present:
		   if (resultpubs[pubs[i]] != nullBitVector):
	               #create a response for publisher i:
		       TMtoPubFID = self.getFIDFromTMToPublisher(pubs[i])
		       outpckt = TMPacket.TmResponse(self.fid_len, 8)
		       outpckt.setType(self.bcon.START_PUBLISH)
	               outpckt.setNofPathIds(inpckt.getNofPathIds())
		       for k in range(0, inpckt.getNofPathIds()):
			   pid = inpckt.getPathId(k);
			   outpckt.setPathId(k, pid, len(pid))
		       #Set FID
		       outpckt.setFIDEntry(resultpubs[pubs[i]])
		       #not placing anything to the outqueue, send this immediatelly
                       print "TMINFO: Notifying Publisher to START_PUBLISH"
		       self.bcon.publishData(array('B',[0xff, 0xff, 0xff, 0xff,0xff, 0xff, 0xff, 0xfd])+array('B',pubs[i]), outpckt.get_packet(), TMtoPubFID, 3)
		   else:
		       #NULL FID Instructs TM to send STOP_PUBLISH
	               #create a response for publisher i:
		       TMtoPubFID = self.getFIDFromTMToPublisher(pubs[i])
		       outpckt = TMPacket.TmResponse(self.fid_len, 8)
		       outpckt.setType(self.bcon.STOP_PUBLISH)
		       outpckt.setNofPathIds(inpckt.getNofPathIds())
		       for k in range(0, inpckt.getNofPathIds()):
			   pid = inpckt.getPathId(k);
			   outpckt.setPathId(k, pid, len(pid))
		       #not placing anything to the outqueue, send this immediatelly PUBLISH_NOW
		       print "TMINFO: Notifying Publisher to STOP_PUBLISH"
		       self.bcon.publishData(array('B',[0xff, 0xff, 0xff, 0xff,0xff, 0xff, 0xff, 0xfd])+array('B',pubs[i]), outpckt.get_packet(), TMtoPubFID, 3)	
        else:
	       #Get Only subscriber array. We correctly use the publisher packet retrieval functions 
	       #Because there are no publishers present, so the publisher head is empty.
	       for i in range(0, inpckt.getNofPublishers()):
                   subs.append(inpckt.getPublisher(i))
               for i in range(0, len(subs)):
	           #create a response for subscribers:
                   TMtoPubFID = self.getFIDFromTMToPublisher(subs[i])
                   outpckt = TMPacket.TmResponse(self.fid_len, 8)
                   outpckt.setType(inpckt.getRequestType())
		   pcktids = inpckt.getNofPathIdsSpecial() 
                   outpckt.setNofPathIds(inpckt.getNofPathIdsSpecial())
                   for k in range(0, pcktids):
	               pid = inpckt.getPathIdSpecial(k);
	               outpckt.setPathId(k, pid, len(pid))
		   print "TMINFO: Notifying Subscribers\n"
	           self.bcon.publishData(array('B',[0xff, 0xff, 0xff, 0xff,0xff, 0xff, 0xff, 0xfd])+array('B',subs[i]), outpckt.get_packet(), TMtoPubFID, 3)

			
    #main_loop is mandated because BlackAdderTMCore starts this as a thread
    def main_loop(self):
	'''TMIgraph main loop'''
	try:
		while True:
	    		print "TMINFO: TIgraph Engine awaits next request"
	    		newpacket = self.GetPacket()
	    		self.HandleTMIGraphPacket(newpacket)
	except  KeyboardInterrupt:
		print "AAAAAA"
	    
	  
