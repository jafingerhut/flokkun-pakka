CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

all: hypercut

hypercut: hypercut.h hypercut.c trie.h trie.c dheap.h dheap.c list.h list.c stdinc.h
	${CPP} ${CFLAGS} -o hypercut hypercut.c trie.c dheap.c list.c

clean:
	/bin/rm -f hypercut
