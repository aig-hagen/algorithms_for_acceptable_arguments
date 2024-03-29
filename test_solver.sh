#!/bin/bash

# Check if ALGORITHM argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <instance.tgf> "
    exit 1
fi

INSTANCE=$1

ARGS="-p EC-PR -fo tgf -f $INSTANCE"
echo "Testing enumeration of credulously accepted arguments..."

echo -n "Testing Solver for IAQ and CrMiSat..."
$(dirname "$0")/"build/bin/solver_IAQ_cmsat" $ARGS
echo -n "Testing Solver for EEE and CrMiSat..."
$(dirname "$0")/"build/bin/solver_EEE_cmsat" $ARGS
echo -n "Testing Solver for SEE and CrMiSat..."
$(dirname "$0")/"build/bin/solver_SEE_cmsat" $ARGS

echo -n "Testing Solver for IAQ and CaDiCal..."
$(dirname "$0")/"build/bin/solver_IAQ_cadical" $ARGS
echo -n "Testing Solver for EEE and CaDiCal..."
$(dirname "$0")/"build/bin/solver_EEE_cadical" $ARGS
echo -n "Testing Solver for SEE and CaDiCal..."
$(dirname "$0")/"build/bin/solver_SEE_cadical" $ARGS

echo -n "Testing Solver for IAQ and Glucose..."
$(dirname "$0")/"build/bin/solver_IAQ_glucose" $ARGS
echo -n "Testing Solver for EEE and Glucose..."
$(dirname "$0")/"build/bin/solver_EEE_glucose" $ARGS
echo -n "Testing Solver for SEE and Glucose..."
$(dirname "$0")/"build/bin/solver_SEE_glucose" $ARGS

echo -n "Testing Solver for IAQ and CGSS2....."
$(dirname "$0")/"build/bin/solver_IAQ_cgss2" $ARGS
echo -n "Testing Solver for EEE and CGSS2....."
$(dirname "$0")/"build/bin/solver_EEE_cgss2" $ARGS
echo -n "Testing Solver for SEE and CGSS2....."
$(dirname "$0")/"build/bin/solver_SEE_cgss2" $ARGS
echo -n "Testing Solver for SEEM and CGSS2...."
$(dirname "$0")/"build/bin/solver_SEEM_cgss2" $ARGS


ARGS="-p ES-PR -fo tgf -f $INSTANCE"
echo "Testing enumeration of skeptically accepted arguments..."

echo -n "Testing Solver for IAQ and CrMiSat..."
$(dirname "$0")/"build/bin/solver_IAQ_cmsat" $ARGS
echo -n "Testing Solver for EEE and CrMiSat..."
$(dirname "$0")/"build/bin/solver_EEE_cmsat" $ARGS
echo -n "Testing Solver for SEE and CrMiSat..."
$(dirname "$0")/"build/bin/solver_SEE_cmsat" -p ES-ST -fo tgf -f $INSTANCE

echo -n "Testing Solver for IAQ and CaDiCal..."
$(dirname "$0")/"build/bin/solver_IAQ_cadical" $ARGS
echo -n "Testing Solver for EEE and CaDiCal..."
$(dirname "$0")/"build/bin/solver_EEE_cadical" $ARGS
echo -n "Testing Solver for SEE and CaDiCal..."
$(dirname "$0")/"build/bin/solver_SEE_cadical" -p ES-ST -fo tgf -f $INSTANCE

echo -n "Testing Solver for IAQ and Glucose..."
$(dirname "$0")/"build/bin/solver_IAQ_glucose" $ARGS
echo -n "Testing Solver for EEE and Glucose..."
$(dirname "$0")/"build/bin/solver_EEE_glucose" $ARGS
echo -n "Testing Solver for SEE and Glucose..."
$(dirname "$0")/"build/bin/solver_SEE_glucose" -p ES-ST -fo tgf -f $INSTANCE

echo -n "Testing Solver for IAQ and CGSS2....."
$(dirname "$0")/"build/bin/solver_IAQ_cgss2" $ARGS
echo -n "Testing Solver for EEE and CGSS2....."
$(dirname "$0")/"build/bin/solver_EEE_cgss2" $ARGS
echo -n "Testing Solver for SEE and CGSS2....."
$(dirname "$0")/"build/bin/solver_SEE_cgss2" -p ES-ST -fo tgf -f $INSTANCE
echo -n "Testing Solver for SEEM and CGSS2...."
$(dirname "$0")/"build/bin/solver_SEEM_cgss2" -p ES-ST -fo tgf -f $INSTANCE