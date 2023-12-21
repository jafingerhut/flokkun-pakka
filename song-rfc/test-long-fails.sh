#! /bin/bash

# A longer running set of tests of rfc program.

# All of these tests fail for reasons that are spelled out in explicit
# error messages when the rfc program exits.  For each of them, it is
# either because MAXEQIDS of 50,000 is not large enough, or MAXTABLE
# of 5,000,000 is not large enough.

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
