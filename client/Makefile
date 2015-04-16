#

#PARAM
CC 		= g++
CFLAGS 	= -g -c -Wall -fPIC -I../
LIB 	= /usr/local/lib/libprotobuf.a
OBJS 	= HomeClient.o ../proto/Log.o ../proto/Message.pb.o

#.PHONY
.PHONY : all client clean

all : client clean

client : $(OBJS)
	$(CC) $(GPERF) -o $@ $(OBJS) $(LIB)

clean : 
	rm -fr $(OBJS)

#OBJS
$(OBJS) : %.o : %.cc
	$(CC) $(CFLAGS) $(GPERF) -o $@ $<
