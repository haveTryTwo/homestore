#

#PARAM
CC 		= g++
CFLAGS 	= -g -c -Wall -fPIC -pthread -I/usr/local/libevent/include -I../ -I/usr/local/include
LIB 	= /usr/local/libevent/lib/libevent.a /usr/local/lib/libprotobuf.a\
		  /usr/local/lib/libleveldb.a
OBJS 	= HomeServer.o Util.o LevelDbImpl.o ../proto/Log.o ../proto/Message.pb.o Main.o

#.PHONY
.PHONY : all server clean

all : server clean

server : $(OBJS)
	$(CC) $(GPERF) -o $@ $(OBJS) -lpthread $(LIB) -lrt

clean : 
	rm -fr $(OBJS)

#OBJS
$(OBJS) : %.o : %.cc
	$(CC) $(CFLAGS) $(GPERF) -o $@ $<
