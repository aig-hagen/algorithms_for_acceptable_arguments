#!/bin/bash

# Check if ALGORITHM argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <iaq|eee|see> <cmsat|cadical|glucose|ext> [solver options]"
    exit 1
fi

# Check if SAT argument is provided
if [ -z "$2" ]; then
    echo "Usage: $0 <iaq|eee|see> <cmsat|cadical|glucose|ext> [solver options]"
    exit 1
fi

ALGORITHM=$1
SAT=$2

BINARY="build/bin/solver_"${ALGORITHM^^}"_$SAT"

if [ ! -f "$BINARY" ]; then
    echo "Binary not found: $BINARY"
    exit 1
fi

shift
shift
$(dirname "$0")/"$BINARY" -s $(dirname "$0")/lib/cadical-1.9.5/build/cadical "$@"
#-s $(dirname "$0")/lib/cryptominisat-5.11.4/build/cryptominisat5