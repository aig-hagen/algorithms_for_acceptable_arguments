#!/bin/bash

# Check if ALGORITHM argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <instance.tgf> "
    exit 1
fi

INSTANCE=$1
SEMANTICS=PR

ARGS="-p EC-$SEMANTICS -fo tgf -f $INSTANCE"
echo "Testing enumeration of credulously accepted arguments for $SEMANTICS... "

echo -n "Testing Solver for IAQ   and CrMiSat... "
$(dirname "$0")/"build/bin/solver_IAQ_cmsat" $ARGS
echo -n "Testing Solver for EEE   and CrMiSat... "
$(dirname "$0")/"build/bin/solver_EEE_cmsat" $ARGS
echo -n "Testing Solver for SEE   and CrMiSat... "
$(dirname "$0")/"build/bin/solver_SEE_cmsat" $ARGS

echo -n "Testing Solver for IAQ   and CaDiCal... "
$(dirname "$0")/"build/bin/solver_IAQ_cadical" $ARGS
echo -n "Testing Solver for EEE   and CaDiCal... "
$(dirname "$0")/"build/bin/solver_EEE_cadical" $ARGS
echo -n "Testing Solver for SEE   and CaDiCal... "
$(dirname "$0")/"build/bin/solver_SEE_cadical" $ARGS

echo -n "Testing Solver for IAQ   and Glucose... "
$(dirname "$0")/"build/bin/solver_IAQ_glucose" $ARGS
echo -n "Testing Solver for EEE   and Glucose... "
$(dirname "$0")/"build/bin/solver_EEE_glucose" $ARGS
echo -n "Testing Solver for SEE   and Glucose... "
$(dirname "$0")/"build/bin/solver_SEE_glucose" $ARGS

echo -n "Testing Solver for IAQ   and CGSS2..... "
$(dirname "$0")/"build/bin/solver_IAQ_cgss2" $ARGS
echo -n "Testing Solver for EEE   and CGSS2..... "
$(dirname "$0")/"build/bin/solver_EEE_cgss2" $ARGS
echo -n "Testing Solver for SEE   and CGSS2..... "
$(dirname "$0")/"build/bin/solver_SEE_cgss2" $ARGS
echo -n "Testing Solver for SEEM  and CGSS2..... "
$(dirname "$0")/"build/bin/solver_SEEM_cgss2" $ARGS


ARGS="-p ES-$SEMANTICS -fo tgf -f $INSTANCE"
echo "Testing enumeration of skeptically accepted arguments for $SEMANTICS... "

echo -n "Testing Solver for IAQ   and CrMiSat... "
$(dirname "$0")/"build/bin/solver_IAQ_cmsat" $ARGS
echo -n "Testing Solver for EEE   and CrMiSat... "
$(dirname "$0")/"build/bin/solver_EEE_cmsat" $ARGS
echo -n "Testing Solver for SEE   and CrMiSat... "
$(dirname "$0")/"build/bin/solver_SEE_cmsat" $ARGS
echo -n "Testing Solver for FUDGE and CrMiSat... "
$(dirname "$0")/"build/bin/solver_FUDGE_cmsat" $ARGS

echo -n "Testing Solver for IAQ   and CaDiCal... "
$(dirname "$0")/"build/bin/solver_IAQ_cadical" $ARGS
echo -n "Testing Solver for EEE   and CaDiCal... "
$(dirname "$0")/"build/bin/solver_EEE_cadical" $ARGS
echo -n "Testing Solver for SEE   and CaDiCal... "
$(dirname "$0")/"build/bin/solver_SEE_cadical" $ARGS
echo -n "Testing Solver for FUDGE and CaDiCal... "
$(dirname "$0")/"build/bin/solver_FUDGE_cadical" $ARGS

echo -n "Testing Solver for IAQ   and Glucose... "
$(dirname "$0")/"build/bin/solver_IAQ_glucose" $ARGS
echo -n "Testing Solver for EEE   and Glucose... "
$(dirname "$0")/"build/bin/solver_EEE_glucose" $ARGS
echo -n "Testing Solver for SEE   and Glucose... "
$(dirname "$0")/"build/bin/solver_SEE_glucose" $ARGS
echo -n "Testing Solver for FUDGE and Glucose... "
$(dirname "$0")/"build/bin/solver_FUDGE_glucose" $ARGS

echo -n "Testing Solver for IAQ   and CGSS2..... "
$(dirname "$0")/"build/bin/solver_IAQ_cgss2" $ARGS
echo -n "Testing Solver for EEE   and CGSS2..... "
$(dirname "$0")/"build/bin/solver_EEE_cgss2" $ARGS
echo -n "Testing Solver for SEE   and CGSS2..... "
$(dirname "$0")/"build/bin/solver_SEE_cgss2" $ARGS
echo -n "Testing Solver for SEEM  and CGSS2..... "
$(dirname "$0")/"build/bin/solver_SEEM_cgss2" $ARGS
echo -n "Testing Solver for FUDGE and CGSS2..... "
$(dirname "$0")/"build/bin/solver_FUDGE_cgss2" $ARGS