#!/bin/bash

# Check if ALGORITHM argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <instance.tgf> "
    exit 1
fi

INSTANCE=$1
ARGS="-p ES-PR -fo tgf -f $INSTANCE"
echo "Testing enumeration of skeptically accepted arguments..."

#echo -n "Testing Solver for IAQ and CrMiSat..."
#$(dirname "$0")/"build/bin/solver_IAQ_cmsat" $ARGS
#echo -n "Testing Solver for EEE and CrMiSat..."
#$(dirname "$0")/"build/bin/solver_EEE_cmsat" $ARGS
#echo -n "Testing Solver for FUDGE and CrMiSat..."
#$(dirname "$0")/"build/bin/solver_FUDGE_cmsat" $ARGS

echo -n "Testing Solver for IAQ and CaDiCal..."
$(dirname "$0")/"build/bin/solver_IAQ_cadical" $ARGS
echo -n "Testing Solver for EEE and CaDiCal..."
$(dirname "$0")/"build/bin/solver_EEE_cadical" $ARGS
echo -n "Testing Solver for FUDGE and CaDiCal..."
$(dirname "$0")/"build/bin/solver_FUDGE_cadical" $ARGS

echo -n "Testing Solver for IAQ and Glucose..."
$(dirname "$0")/"build/bin/solver_IAQ_glucose" $ARGS
echo -n "Testing Solver for EEE and Glucose..."
$(dirname "$0")/"build/bin/solver_EEE_glucose" $ARGS
echo -n "Testing Solver for FUDGE and Glucose..."
$(dirname "$0")/"build/bin/solver_FUDGE_glucose" $ARGS

echo -n "Testing Solver for IAQ and CGSS2....."
$(dirname "$0")/"build/bin/solver_IAQ_cgss2" $ARGS
echo -n "Testing Solver for EEE and CGSS2....."
$(dirname "$0")/"build/bin/solver_EEE_cgss2" $ARGS
echo -n "Testing Solver for FUDGE and CGSS2....."
$(dirname "$0")/"build/bin/solver_FUDGE_cgss2" $ARGS