all: pgid

pgid: pgid.o
	g++ -o pgid pgid.o

pgid.o: pgid.c
	g++ -c pgid.c
