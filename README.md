# algorithms_for_acceptable_arguments v1.0 (03-2024)

Implements several SAT-based algorithms for enumerating credulously/skeptically accepted arguments for abstract argumentation frameworks.

Supports the following problems: [`EC-CO`,`EC-PR`,`EC-ST`,`ES-PR`,`ES-ST`]

Supports TGF and i23 format for abstract argumentation frameworks.


## Installation

### Building SAT Solvers
This solver supports multiple integrated SAT Solvers, namely [CaDiCal](https://github.com/arminbiere/cadical) (version 1.9.5), [Glucose](https://github.com/audemard/glucose) (version 4.2.1) and [CryptoMiniSat](https://github.com/msoos/cryptominisat) (version 5.11.4).
The relevant source files are included and the solvers can easily be built as follows

#### CaDiCal
```
  make cadical
```

#### Glucose
Glucose requires `cmake`
```
  sudo apt-get install build-essential cmake
```
Building Glucose
```
  make glucose
```

### CryptoMiniSat
CryptoMiniSat requires `cmake`
```
  sudo apt-get install build-essential cmake
```
Install Boost libraries and other prerequisites
```
sudo apt-get install zlib1g-dev libboost-program-options-dev libsqlite3-dev
```

Building CryptoMiniSat
```
  make cmsat
```

CryptoMiniSat must also be installed
```
  cd lib/cryptominisat-5.11.04/build
  sudo make install
  sudo ldconfig
```

### Compile and Build Solver
To compile and build solver for all algorithms use
```
    make all
```

Alternatively, build only for a specific algorithm via
```
    make <iaq|eee|see|seem|fudge>
```
or 
```
    make ALGORITHM=<iaq|eee|see|seem|fudge>
```

The SAT solver to be used can be specified as follows
```
  make <iaq|eee|see|seem|fudge> SAT_SOLVER=<cadical|glucose|cryptominisat>
```

#### Example
To build only the solver based on the IAQ algorithm using the CaDiCal SAT Solver
```
  make iaq SAT_SOLVER=cadical
```
Or, to build the solver for all algorithms using Glucose
```
  make all SAT_SOLVER=glucose
```

## Command-line usage

```
./solver.sh <algorithm> -p <task> -f <file> -fo <format>
  
  <algorithm> the algorithm for computation, one of: [iaq,eee,see,seem,fudge]
  <task>      computational problem
  <file>      input argumentation framework
  <format>    file format for input AF

Options:
  --help      Displays this help message.
  --version   Prints version and author information.
  --formats   Prints available file formats.
  --problems  Prints available computational problems.
```

Example usage:
```
  ./solver.sh iaq -p EC-PR -fo tgf -f <file in TGF format>
```

Alternatively, you can also directly call the binary for each algorithm, e.g.
```
  ./build/bin/<algorithm>/solver -p EC-PR -fo tgf -f <file in TGF format>
```

