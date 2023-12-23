CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

all: rfc

rfc: rfc.c rfc.h dheap.c dheap.h stdinc.h
	${CPP} ${CFLAGS} -o rfc rfc.c dheap.c

clean:
	/bin/rm -f rfc
