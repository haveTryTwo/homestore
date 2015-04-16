#

#PARAM
CC 		= g++
CFLAGS 	= -g -c -Wall -fPIC -pthread
OBJS 	= Message.pb.o Main.o

#.PHONY
.PHONY : all build clean

all : build clean

build : $(OBJS)
	$(CC) $(GPERF) -o $@ $(OBJS)  /usr/local/lib/libprotobuf.a

clean : 
	rm -fr $(OBJS)

#OBJS
$(OBJS) : %.o : %.cc
	$(CC) $(CFLAGS) $(GPERF) -o $@ $<
