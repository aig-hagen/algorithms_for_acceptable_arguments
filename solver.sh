#!/bin/bash

# Check if ALGORITHM argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <iaq|eee|see> [binary options]"
    exit 1
fi

ALGORITHM=$1

BINARY="build/bin/$ALGORITHM/solver"

if [ ! -f "$BINARY" ]; then
    echo "Binary not found for algorithm: $ALGORITHM"
    exit 1
fi

shift
$(dirname "$0")/"$BINARY" -s $(dirname "$0")/lib/cryptominisat-5.11.4/build/cryptominisat5 "$@"
