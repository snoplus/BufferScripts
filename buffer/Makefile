CFLAGS = -Wall -Wextra

LINKFLAGS = -lcurl

all: runtype

runtype: runtype.o
	g++ -o runtype runtype.o $(LINKFLAGS)

runtype.o: runtype.cpp
	g++ -c runtype.cpp $(CFLAGS)
