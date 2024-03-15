# solver v1.0 (03-2024)

Compile via
```
    make
```

Implements the problems [EC-CO,EC-PR,EC-ST,ES-PR,ES-ST] and supports the
TGF and i23 format for abstract argumentation frameworks.

Currently works with an external cryptominisat5 solver.
Can be compiled via 
```
    make cmsat
```

Example usage:
```
  ./solver.sh -p EC-PR -fo tgf -f <file in TGF format>
```
