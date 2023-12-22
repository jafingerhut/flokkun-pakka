CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

all: woo

woo: woo.h woo.c trie.h trie.c dheap.h dheap.c list.h list.c
	${CPP} ${CFLAGS} -o  woo woo.c trie.c dheap.c list.c

clean:
	/bin/rm -f woo
