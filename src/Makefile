CC = g++
CFLAGS = -std=c++11
LDFLAGS = -L/mnt/j/proj/misc/20181119_seqlib/src
LDLIBS = -lhts -lcurl -lcrypto -lz -lpthread -llzma -lbz2 -lseqlib
INC = -I/mnt/j/proj/misc/20181119_seqlib/

GAC: get_all_calls.cpp walker/walker.cpp get_all_calls.hpp walker/walker.hpp
	$(CC) $(EXTRA) $(CFLAGS) walker/argparse.cpp walker/walker.cpp get_all_calls.cpp -o get_all_calls $(LDFLAGS) $(LDLIBS) $(INC)
