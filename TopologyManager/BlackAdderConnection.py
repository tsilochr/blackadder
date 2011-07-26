# -*- coding: utf-8 -*-
# BlackAdderConnection.py Netlink Connection Prequisites. 
# Author: Dimitris Syrivelis
# ITI CERTH for Pursuit project. This source is under BlackAdder license 

import errno
import os
import array
import pwd
import select
import socket
import struct
import sys
import time
import baddernetlink #This is a native object you should have installed baddernetlink-0.1 library to your python distribution

#Sets a global connection handle for TM that will be shared among python threads
global bcon
bcon=None

class Connection:
    '''Implements netlink connections. Do not forget to expand support for unexpected messages'''
    # BlackAdder Action types follow 
    PUBLISH_SCOPE = 0
    PUBLISH_INFO = 1
    UNPUBLISH_SCOPE = 2
    UNPUBLISH_INFO = 3
    SUBSCRIBE_SCOPE = 4
    SUBSCRIBE_INFO = 5
    UNSUBSCRIBE_SCOPE = 6
    UNSUBSCRIBE_INFO = 7
    PUBLISH_DATA = 8
    START_PUBLISH = 100
    STOP_PUBLISH = 101
    SCOPE_PUBLISHED = 102
    SCOPE_UNPUBLISHED = 103
    PUBLISHED_DATA = 104
    #BlackAdder Strategy Types follow
    NODE_LOCAL = 0
    LINK_LOCAL = 1
    DOMAIN_LOCAL = 2
    PUBLISH_NOW = 4
    SUBSCRIBE_LOCALLY = 4

    def __init__(self, baddertype, unexpected_msg_handler = None):
        self.sockfd=baddernetlink.badderconnectioninit(type=baddertype,PursuitIDlen=8)
        self.bvsize = 8

    def add_FID_long_base(self,lida, index, value, order = '='):
        "Set 4-byte 'value' at 'index'. See struct module's documentation to understand the meaning of 'order'."
        ary = array.array("B", struct.pack(order + 'L', value))
        lida[index:index+4] = ary
        return lida
  
    def send(self,mtype,idlength,idstr,prefixlen,prefixidstr,strtg,lidlength,lidstr,datalen,datastr):
	'''A raw version of send - probably it will never be used'''
        baddernetlink.baddersend(sockfd=self.sockfd,type=mtype, idlen=(idlength/8), id=idstr, pidlen=(prefixlen/8), prefixid=prefixidstr, 
							strategy=strtg, LID=lidstr, data=datastr)

    def recv(self):
	'''Returns a tuple: (0:type, 1:idlen, 2:idstring, 3:data=(present only if type == PUBLISHED_DATA))'''
	return baddernetlink.badderrecv(sockfd=self.sockfd);

    def disconnect(self):
        '''Disconnects'''
	return baddernetlink.badderdisconnect(sockfd=self.sockfd);

    def publishAction(self, atype, scopeid, prefixidstr, strtg):
	'''Publish Action'''
	baddernetlink.baddersend(sockfd=self.sockfd,type=atype, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixidstr)/8), prefixid=prefixidstr, 
										strategy=strtg,  LID=None, data=None)
    def publishActionLID(self,atype, scopeid, prefixidstr, strtg, lidstr):
	'''Publishes Actions only'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=atype, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def publishScope(self, scopeid, prefixidstr, strtg, lidstr=None):
	'''Publishes Scope'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=PUBLISH_SCOPE, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def publishInfo(self, scopeid, prefixidstr, strtg, lidstr=None):
	'''Publishes Info'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=PUBLISH_INFO, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def unpublishScope(self, scopeid, prefixidstr, strtg, lidstr=None):
	'''Publishes Actions only'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=UNPUBLISH_SCOPE, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def unpublishInfo(self, scopeid, prefixidstr, strtg, lidstr=None):
	'''Publishes Actions only'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=UNPUBLISH_INFO, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def subscribeScope(self, scopeid, prefixidstr, strtg, lidstr=None):
	'''Publishes Actions only'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=SUBSCRIBE_SCOPE, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def unsubscribeScope(self, scopeid, prefixidstr, strtg, lidstr=None):
	'''Publishes Actions only'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=UNSUBSCRIBE_SCOPE, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def startPublish(self, scopeid, prefixidstr, strtg, lidstr=None):
	'''Publishes Actions only'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=START_PUBLISH, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def stopPublish(self, scopeid, prefixidstr, strtg, lidstr=None):
	'''Publishes Actions only'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=STOP_PUBLISH, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def scopePublished(self, scopeid, prefixidstr, strtg, lidstr=None):
	'''Publishes Actions only'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=SCOPE_PUBLISHED, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def scopeUnpublished(self, scopeid, prefixidstr, strtg, lidstr=None):
	'''Publishes Actions only'''
	baddernetlink.baddersend(sockfd=self.sockfd, type=SCOPE_UNPUBLISHED, idlen=(len(scopeid)/8), id=scopeid, pidlen=(len(prefixid)/8), prefixid=prefixidstr, 
										strategy=strtg, LID=lidstr, data=None)
    def publishData(self,idstr, datastr, fidbv=None, strtg=None):
	'''Publishes Data with LID'''
	if(fidbv):
	    lidarray = array.array('B', '\0' * (self.bvsize / 8))
	    bitvectoruint32t = self.bvsize / 4
	    start = 0
	    for i in range(0, bitvectoruint32t):
		    datachunk = 0x00000000		
		    currentCycle = 32*(bitvectoruint32t - i) - 1
		    for j in range(0, 32):
			    if(fidbv[currentCycle-j] == 1):
				    datachunk = datachunk | (1 << j)
		    lidarray = self.add_FID_long_base(lidarray, start, datachunk)
		    start = start + 4
	    baddernetlink.baddersend(sockfd=self.sockfd,type=self.PUBLISH_DATA, idlen=(len(idstr)/8), id=idstr.tostring(), pidlen=0, prefixid=None, 
										strategy=strtg , LID=lidarray.tostring(), data=datastr)
        else:
	    baddernetlink.baddersend(sockfd=self.sockfd,type=PUBLISH_DATA, idlen=(len(idstr)/8), id=idstr, pidlen=0, prefixid=None, 
										strategy=0,  LID=None,  data=datastr)
      

