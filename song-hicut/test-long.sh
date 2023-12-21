#! /bin/bash

# A longer running set of tests of hicut program.

#CMD="./hicut"
CMD="valgrind --tool=memcheck ./hicut"

# Failing test cases:
# fw1_1K fw1_5K fw1_10K
# ipc1_10K
# Some kind of failure reading uninitialized data that led to a SIGSEGV
# after 5.8 mins of running with valgrind memcheck enabled
# see file out-test-fail-fw1_1K.txt
# see file out-test-fail-fw1_5K.txt
# see file out-test-fail-fw1_10K.txt
# see file out-test-fail-ipc1_10K.txt


set -e
make
for j in \
    acl1_100 acl1_1K acl1_5K acl1_10K \
    fw1_100 \
    ipc1_100 ipc1_1K ipc1_5K
do
    echo ""
    echo "-----------------------------------------------------------------"
    echo $j
    echo "-----------------------------------------------------------------"
    set -x
    time ${CMD} -r ../orig/song-filterset/${j} -t ../orig/song-filterset/${j}_trace
    set +x
done

echo ""
echo "All tests passed without errors."
