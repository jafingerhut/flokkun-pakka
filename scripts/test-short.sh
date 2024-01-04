#! /bin/bash

T="orig/song-filterset"
G="genrules"

mkdir -p tmp
set -e
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
    ./scripts/test_ipv4_classbench_rule_stats.sh ${j} tmp/${k}_rule_stats
    #./scripts/test_ruleio.sh ${j} tmp/${k}
    #diff -iw ${j} tmp/${k}
    #/bin/rm -f ${k}
    #./scripts/test_remove_unmatchable.sh ${j} tmp/${k}_matchable_rules tmp/${k}_unmatchable_rules
    #./scripts/test_remove_duplicates.sh ${j} tmp/${k}_unique_rules tmp/${k}_duplicate_rules
    #./scripts/test_write_og.sh ${j} tmp/${k}_conf.gv
    #./scripts/test_add_resolve_rules.sh ${j} tmp/${k}_addrr
    set +x
done

echo ""
echo "All tests passed without errors."
