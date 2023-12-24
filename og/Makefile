CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

all: og

og: llist.h llist.c og.c og.h stdinc.h
	${CPP} ${CFLAGS} -o og og.c llist.c

clean:
	/bin/rm -f og
