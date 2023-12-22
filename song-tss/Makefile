CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

all: tss

tss: tss.h tss.c trie.h trie.c stdinc.h
	${CPP} ${CFLAGS} -o  tss tss.c trie.c

clean:
	/bin/rm -f tss
