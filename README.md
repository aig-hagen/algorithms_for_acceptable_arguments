# serial-solver v1.0 (02-2023)

Compile via
```
    ./build.sh
```

Implements the problems [DS-PR, DS-UC, EE-UC, EE-IT, CE-IT] and supports the
TGF format for abstract argumentation frameworks. 

Works with an integrated cryptominisat5 solver.

Also supports any external SAT solver that can read dimacs input from <stdin>.
Build the SAT solver seperately and provide the link to the executable in 'serial-solver.sh'.

Example usage:
```
  ./serial-solver.sh -p DS-PR -fo tgf -f <file in TGF format> -a <argument>
```
