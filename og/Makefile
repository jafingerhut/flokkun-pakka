CPP = g++
#CFLAGS = -ggdb -Wall
CFLAGS = -O3 -Wall

all: og addconfrules test_rules test_writerule

og: llist.hpp llist.cpp rules.hpp rules.cpp og.cpp og.hpp stdinc.hpp
	${CPP} ${CFLAGS} -o $@ og.cpp llist.cpp rules.cpp

addconfrules: llist.hpp llist.cpp rules.hpp rules.cpp addconfrules.cpp addconfrules.hpp stdinc.hpp
	${CPP} ${CFLAGS} -o $@ addconfrules.cpp llist.cpp rules.cpp

test_rules: llist.hpp llist.cpp rules.hpp rules.cpp test_rules.cpp
	${CPP} ${CFLAGS} -o $@ test_rules.cpp llist.cpp rules.cpp

test_writerule: llist.hpp llist.cpp rules.hpp rules.cpp test_writerule.cpp
	${CPP} ${CFLAGS} -o $@ test_writerule.cpp llist.cpp rules.cpp

clean:
	/bin/rm -f og addconfrules test_rules test_writerule *.gv
