CPP = g++
#CFLAGS = -ggdb -Wall
#CFLAGS = -O2 -Wall
CFLAGS = -O3 -Wall

rfc: rfc.c rfc.h dheap.c dheap.h stdinc.h
	${CPP} ${CFLAGS} -o rfc rfc.c dheap.c  

all: rfc 
