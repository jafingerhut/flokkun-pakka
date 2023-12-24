CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

all: og addconfrules test_rules test_writerule

og: llist.h llist.c rules.h rules.c og.c og.h stdinc.h
	${CPP} ${CFLAGS} -o $@ og.c llist.c rules.c

addconfrules: llist.h llist.c rules.h rules.c addconfrules.c addconfrules.h stdinc.h
	${CPP} ${CFLAGS} -o $@ addconfrules.c llist.c rules.c

test_rules: llist.h llist.c rules.h rules.c test_rules.c
	${CPP} ${CFLAGS} -o $@ test_rules.c llist.c rules.c

test_writerule: llist.h llist.c rules.h rules.c test_writerule.c
	${CPP} ${CFLAGS} -o $@ test_writerule.c llist.c rules.c

clean:
	/bin/rm -f og addconfrules test_rules test_writerule
