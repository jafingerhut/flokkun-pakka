#! /bin/bash

# A longer running set of tests of rfc program.

# TODO: This failed on file fw1_1K the last time I tried it, with a
# segmentation fault.  Debug this to figure out what is going wrong.

#CMD="./rfc"
CMD="valgrind --tool=memcheck ./rfc"

set -e
make
for j in \
    acl1_100 acl1_1K acl1_5K acl1_10K \
    fw1_100 fw1_1K \
    ipc1_100 ipc1_1K
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
