#! /bin/bash

# A longer running set of tests of rfc program.

# TODO: This failed on file fw1_1K the last time I tried it, with a
# segmentation fault.  Debug this to figure out what is going wrong.

CMD="./rfc"

make
for j in \
    fw1_5K fw1_10K \
    ipc1_5K ipc1_10K
do
    echo ""
    echo "-----------------------------------------------------------------"
    echo $j
    echo "-----------------------------------------------------------------"
    set -x
    ${CMD} -r ../orig/song-filterset/${j} -t ../orig/song-filterset/${j}_trace
    exit_status=$?
    if [ ${exit_status} -eq 0 ]
    then
	echo "Expected test ${j} to fail, but it succeeded."
	exit 1
    fi
    set +x
done

echo ""
echo "All tests failed, as expected."
