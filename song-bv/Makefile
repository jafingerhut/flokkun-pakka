CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

bv: bv.h bv.c bst.h bst.c dheap.h dheap.c list.h list.c stdinc.h
	${CPP} ${CFLAGS} -o  bv bv.c bst.c dheap.c list.c 

all: bv 
