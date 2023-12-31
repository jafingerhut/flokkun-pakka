#! /bin/bash

# A quick test of the bv program using a small ACL and set of test
# packets.

#CMD="./bv"
CMD="valgrind --tool=memcheck ./bv"

set -e
make
#${CMD} -r ../orig/song-filterset/acl1_100 -t acl1_100_trace.with-intentional-error
for j in acl1_100 fw1_100 ipc1_100
do
    echo ""
    echo "-----------------------------------------------------------------"
    echo $j
    echo "-----------------------------------------------------------------"
    set -x
    ${CMD} -r ../orig/song-filterset/${j} -t ../orig/song-filterset/${j}_trace
    set +x
done

set +x
echo ""
echo "All tests passed without errors."
