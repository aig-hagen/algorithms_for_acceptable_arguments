# algorithms_for_acceptable_arguments v1.0 (03-2024)

Implements several SAT-based algorithms for enumerating credulously/skeptically accepted arguments for abstract argumentation frameworks.

Supports the following problems: [`EC-CO`,`EC-PR`,`EC-ST`,`ES-PR`,`ES-ST`]

Supports TGF and i23 format for abstract argumentation frameworks.

## Dependencies
- libboost
- cryptominisat-5.11.04


## Installation

### Build and Install Cryptominsat5
Cryptominisat requires `cmake`
```
  sudo apt-get install build-essential cmake
```

Install Boost libraries and other prerequisites
```
sudo apt-get install zlib1g-dev libboost-program-options-dev libsqlite3-dev
```

Building cryptominisat
```
  make cmsat
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

