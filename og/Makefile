CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

all: og

og: og.c og.h stdinc.h
	${CPP} ${CFLAGS} -o og og.c

clean:
	/bin/rm -f og
