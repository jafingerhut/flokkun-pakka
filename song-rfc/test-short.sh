#! /bin/bash

# A quick test of the rfc program using a small ACL and set of test
# packets.

set -ex
make
#./rfc -r ../orig/song-filterset/acl1_100 -t acl1_100_trace.with-intentional-error
for j in acl1_100 fw1_100 ipc1_100
do
    ./rfc -r ../orig/song-filterset/${j} -t ../orig/song-filterset/${j}_trace
done

set +x
echo ""
echo "All tests passed without errors."