# CGSS2 MaxSAT solver


CGSS2 [[1]](#1) is a C++ implementation of OLL for solving MaxSAT.


It implements same techniques (and some extra stuff) as [CGSS](https://bitbucket.org/coreo-group/cgss/src/master/) [[2]](#2).

### Building and running

#### Dependencies:

Ability to read .gz and .xz files requires boost. This can be disabled by removing lines `CFLAGS += -DZLIB` and  `LIBFILES +=  -lboost_iostreams` from `src/Makefile`.

To init submodules (MaxPre 2 [[3](#3),[4](#4)] and CaDiCaL) and prepare CaDiCaL for compiling run

```./init_submodules.sh```


This runs first:

```git submodule update --init --recursive```

And then prepares CaDiCaL

```
cd cadical && ./configure
```

Compiling with MaxPre 2 and/or CaDiCaL can be disabled by removing the relevant lines from `src/Makefile`.


#### Compile:

```make```

#### Running:
```/cgss2 [parameters] instance.wcnf[.gz|.xz] [parameters]```

### References

<a id="1">[1]</a> H. Ihalainen. *“Refined core relaxations for core-guided maximum satisfiability algorithms”*.
MSc thesis, University of Helsinki, 2022.
url: [https://hdl.handle.net/10138/351207](https://hdl.handle.net/10138/351207).

<a id="2">[2]</a>
H. Ihalainen, J. Berg, and M. Järvisalo. *“Refined Core Relaxation for Core-Guided MaxSAT Solving”*.
In: CP 2021, pp 28:1–28:19
url: [https://doi.org/10.4230/LIPIcs.CP.2021.28](https://doi.org/10.4230/LIPIcs.CP.2021.28).

<a id="3">[3]</a>
Ihalainen, Hannes, Jeremias Berg, and Matti Järvisalo. *"Clause Redundancy and Preprocessing in Maximum Satisfiability."*
In: IJCAR 2022. pp. 75-94
url: [https://doi.org/10.1007/978-3-031-10769-6_6](https://doi.org/10.1007/978-3-031-10769-6_6).

<a id="4">[4]</a>
Korhonen, T., Berg, J., Saikko, P., & Järvisalo, M. (2017, August). MaxPre: an extended MaxSAT preprocessor.
In: SAT 2017, pp. 449-456
url: [https://doi.org/10.1007/978-3-319-66263-3_28](https://doi.org/10.1007/978-3-319-66263-3_28)
