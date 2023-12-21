CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

hicut: hicut.h hicut.c trie.h trie.c dheap.h dheap.c list.h list.c stdinc.h
	${CPP} ${CFLAGS} -o  hicut hicut.c trie.c dheap.c list.c 

all: hicut
