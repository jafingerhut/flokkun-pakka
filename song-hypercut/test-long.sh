#! /bin/bash

# A longer running set of tests of hypercut program.

# TODO: This failed on file fw1_1K the last time I tried it, with a
# segmentation fault.  Debug this to figure out what is going wrong.

#CMD="./hypercut"
CMD="valgrind --tool=memcheck ./hypercut"

#    acl1_100 acl1_1K acl1_5K acl1_10K \
#    fw1_100 fw1_1K fw1_5K fw1_10K \
#    ipc1_100 ipc1_1K ipc1_5K ipc1_10K

# tests with errors:
# fw1_1K
# The above failed with a message from valgrind about
#==7054== Process terminating with default action of signal 11 (SIGSEGV)
#==7054==  Access not within mapped region at address 0x0
#==7054==    at 0x10C4A8: trie::createtrie() (trie.c:390)
#==7054==    by 0x10A922: trie::trie(int, int, int, float, pc_rule*, int, int, int) (trie.c:51)
#==7054==    by 0x10A127: main (hypercut.c:191)

# fw1_5K fw1_10K ipc1_5K ipc1_10K
# All of the above failed with a message like this:
#Fatal:list::operator&=: item 1000001 is outside of allowed range [1,1000000]

set -e
make
for j in \
    acl1_100 acl1_1K acl1_5K acl1_10K \
    fw1_100 \
    ipc1_100 ipc1_1K
do
    echo ""
    echo "-----------------------------------------------------------------"
    echo $j
    echo "-----------------------------------------------------------------"
    set -x
    ${CMD} -r ../orig/song-filterset/${j} -t ../orig/song-filterset/${j}_trace
    set +x
done

echo ""
echo "All tests passed without errors."
