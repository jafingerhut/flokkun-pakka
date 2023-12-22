CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

all: hicut

hicut: hicut.h hicut.c trie.h trie.c dheap.h dheap.c list.h list.c stdinc.h
	${CPP} ${CFLAGS} -o  hicut hicut.c trie.c dheap.c list.c

clean:
	/bin/rm -f hicut
