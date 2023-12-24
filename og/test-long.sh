#! /bin/bash

CMD="./og"
#CMD="valgrind --tool=memcheck ./og"

T="../orig/song-filterset"
G="../genrules"

set -e
make
./test_rules
for j in \
    $T/acl1 $T/fw1 $T/ipc1 \
    $T/acl1_100 $T/fw1_100 $T/ipc1_100 \
    $T/acl1_1K $T/fw1_1K $T/ipc1_1K \
    $T/acl1_5K $T/fw1_5K $T/ipc1_5K \
    $T/acl1_10K $T/fw1_10K $T/ipc1_10K \
    $G/bc1_k5 $G/bc1_k40 $G/bc1_k100
do
    k=`basename $j`
    echo ""
    echo "-----------------------------------------------------------------"
    echo $j
    echo "-----------------------------------------------------------------"
    set -x
    time ${CMD} -r ${j} > ${k}_conf.gv
    ./test_writerule -r ${j} -w ${k}
    diff -iw ${j} ${k}
    /bin/rm -f ${k}
    set +x
done

echo ""
echo "All tests passed without errors."
