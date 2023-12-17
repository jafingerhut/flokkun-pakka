#! /bin/bash

# A longer running set of tests of rfc program.

# TODO: This failed on file fw1_1K the last time I tried it, with a
# segmentation fault.  Debug this to figure out what is going wrong.

set -ex
make
for j in \
    acl1_100 acl1_1K acl1_5K acl1_10K \
    fw1_100 fw1_1K fw1_5K fw1_10K \
    ipc1_100 ipc1_1K ipc1_5K ipc1_10K
do
    ./rfc -r ../orig/song-filterset/${j} -t ../orig/song-filterset/${j}_trace
done

set +x
echo ""
echo "All tests passed without errors."
