#! /bin/bash

print_usage() {
    1>&2 echo "usage: $0 <infile> <outfile-unique-rules> <outfile-duplicate-rules>"
}

if [ $# -ne 3 ]
then
    print_usage
    exit 1
fi

IN="$1"
OUT="$2"
OUT2="$3"

clojure -X:test-remove-duplicates "{:in \"${IN}\" :out \"${OUT}\" :out-unmatchable \"${OUT2}\"}"
