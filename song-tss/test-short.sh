#! /bin/bash

# A quick test of the tss program using a small ACL and set of test
# packets.

#CMD="./tss"
CMD="valgrind --tool=memcheck ./tss"

set -e
make
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

echo ""
echo "All tests passed without errors."
