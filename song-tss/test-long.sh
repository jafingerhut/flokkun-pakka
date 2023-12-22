#! /bin/bash

# A longer running set of tests of tss program.

CMD="./tss"
#CMD="valgrind --tool=memcheck ./tss"

set -e
make
for j in \
    acl1_100 ipc1_100 fw1_100 \
    acl1_1K  ipc1_1K  fw1_1K  \
    acl1_5K  ipc1_5K  fw1_5K  \
    acl1_10K ipc1_10K fw1_10K
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
