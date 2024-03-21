#!/bin/bash

ARGS="-p EC-PR -fo tgf -f example.tgf"
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



ARGS="-p ES-PR -fo tgf -f example.tgf"
echo "Testing enumeration of skeptically accepted arguments..."

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