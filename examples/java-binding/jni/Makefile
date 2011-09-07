CC = g++
JVI_INCL= -I/usr/lib/jvm/java-1.6.0-openjdk/include

all: BlackadderWrapper.o BlackadderWrapper.so

BlackadderWrapper.o: 
	$(CC) -shared -g $(JVI_INCL) -fPIC -o eu_pursuit_client_BlackadderWrapper.o eu_pursuit_client_BlackadderWrapper.cc blackadder.cpp bitvector.cpp

BlackadderWrapper.so: 
	$(CC) -shared -o eu_pursuit_client_BlackadderWrapper.so eu_pursuit_client_BlackadderWrapper.o
