#! /bin/bash

print_usage() {
    1>&2 echo "usage: $0 <infile> <outfile>"
}

if [ $# -ne 2 ]
then
    print_usage
    exit 1
fi

IN="$1"
OUT="$2"

clojure -X:test-rule-io "{:in \"${IN}\" :out \"${OUT}\"}"
