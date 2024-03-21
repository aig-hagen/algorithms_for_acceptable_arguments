GITV = $(shell git describe --always --dirty)
CC = g++ 
CFLAGS += -std=c++11 -O3 -Wall -Wextra -Wshadow -g -Wfatal-errors  -Lsolvers/lib/ -DGIT_IDENTIFIER=\"$(GITV)\"
OBJFILES = cgss2.o options.o ss_encoder.o satsolver.o clausebuffer.o optimizer.o
LIBS =
STATICLIBS = -static
LIBFILES =
LIBOBJFILES =


with_zlib = 'true'
with_gurobi = 'false'
with_cplex = 'false'
with_scip = 'false'
with_cadical = 'true'
with_glucose3 = 'true'
with_glucose3x = 'true'
with_maxpre2 = 'false'

ifeq ($(with_zlib),'true')
CFLAGS += -DZLIB
LIBS +=  -lboost_iostreams
STATICLIBS +=  -l:libboost_iostreams.a -l:libz.a -l:liblzma.a
endif

ifeq ($(with_gurobi),'true')
CFLAGS += -DWITH_GUROBI
OBJFILES += optimizers/gurobi/gurobi_optimizer.o
LIBFILES +=
LIBS += -lgurobi_c++ -lgurobi100
STATICLIBS += -l:libgurobi_c++.a -l:libgurobi100.a
#LIBS += -lgurobi_c++ -lgurobi95
ipamir: gurobi
solver: gurobi
lib: gurobi
clean-all: clean-gurobi
endif

# cplex
ifeq ($(with_cplex),'true')
CFLAGS += -DWITH_CPLEX -DIL_STD
OBJFILES += optimizers/cplex/cplex_optimizer.o
LIBFILES +=
LIBS +=  -lconcert -lilocplex -lcplex -lpthread -ldl
STATICLIBS +=  -l:libconcert.a -l:libilocplex.a -l:libcplex.a -l:libpthread.a -l:libdl.a
ipamir: cplex
solver: cplex
lib: cplex
clean-all: clean-cplex
endif

ifeq ($(with_scip),'true')
CFLAGS += -DWITH_SCIP 
OBJFILES += optimizers/scip/scip_optimizer.o
LIBFILES +=
LIBS += -lscipbase -llpispx2 -ltpinone -lsoplex
STATICLIBS += -l:libscipbase.a -l:liblpispx2.a -l:libtpinone.a -l:libsoplex.a
ipamir: scip
solver: scip
lib: scip
clean-all: clean-scip
endif


# cadical
ifeq ($(with_cadical),'true')
CFLAGS += -DCADICAL
OBJFILES += solvers/cadical/cadical_solver.o
LIBFILES +=  libcadical.a
LIBOBJFILES += ../cadical/build/*.o
LIBS += -lcadical
STATICLIBS += -l:libcadical.a
ipamir: cadical
solver: cadical
lib: cadical
clean-all: clean-cadical
endif

# glucose
ifeq ($(with_glucose3),'true')
CFLAGS += -DGLUCOSE3
OBJFILES += solvers/glucose3/glucose3_solver.o
LIBFILES += libglucose3.a
LIBOBJFILES += solvers/glucose3/core/Solver.o  solvers/glucose3/utils/System.o
LIBS += -lglucose3
STATICLIBS += -l:libglucose3.a
ipamir: glucose3
solver: glucose3
lib: glucose3
clean-all: clean-glucose3
endif

# glucose 3x
ifeq ($(with_glucose3x),'true')
CFLAGS += -DGLUCOSE3X
OBJFILES += solvers/glucose3x/glucose3x_solver.o
LIBFILES += libglucose3x.a
LIBOBJFILES += solvers/glucose3x/core/Solver.o  solvers/glucose3x/utils/System.o
LIBS += -lglucose3x
STATICLIBS += -l:libglucose3x.a
ipamir: glucose3x
solver: glucose3x
lib: glucose3x
clean-all: clean-glucose3x
endif


# MAXPRE2
ifeq ($(with_maxpre2),'true')
CFLAGS += -DMAXPRE  -L../maxpre2/src/lib/ -I../maxpre2/src
LIBFILES += libmaxpre.a
LIBS += -lmaxpre
STATICLIBS += -l:libmaxpre.a 
solver: maxpre
lib: maxpre
clean-all: clean-maxpre
endif

all: solver

gurobi:

cplex:

scip:

solver: main.o  interface.o dimacs.o $(OBJFILES)
	$(CC) $(CFLAGS) main.o interface.o dimacs.o $(OBJFILES) $(LIBS) -o cgss2

static: main.o  interface.o dimacs.o $(OBJFILES)
	$(CC) $(CFLAGS) main.o interface.o dimacs.o $(OBJFILES) $(STATICLIBS) -o cgss2

.PHONY: lib

lib: interface.o dimacs.o $(OBJFILES)
	ar rvs lib/libcgss2.a interface.o dimacs.o $(OBJFILES) $(LIBOBJFILES)

sipamir: ipamir
	ar

ipamir: ipamir.o $(OBJFILES)
	mkdir -p lib
	ar rvs lib/libipamircgss2.a ipamir.o $(OBJFILES) $(LIBOBJFILES)

glucose3:
	$(MAKE) -C solvers/glucose3 lib
	cp solvers/lib/libglucose3.a lib/libglucose3.a

cadical:
	$(MAKE) -C ../cadical
	cp ../cadical/build/libcadical.a solvers/lib/libcadical.a
	cp ../cadical/build/libcadical.a lib/libcadical.a

glucose3x:
	$(MAKE) -C solvers/glucose3x lib
	cp solvers/lib/libglucose3x.a lib/libglucose3x.a

maxpre:
	$(MAKE) -C ../maxpre2 lib
	mkdir -p lib
	cp ../maxpre2/src/lib/libmaxpre.a lib/libmaxpre.a


%.o: %.cpp
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include *.d


clean-all: clean

clean: clean-cgss2

clean-cgss2:
	rm -f lib/*.a
	rm -f *.o *.d
	rm cgss2
	rm ../cgss2
	
clean-gurobi:
	rm -f optimizers/gurobi/*.o
	rm -f optimizers/gurobi/*.d

clean-cplex:
	rm -f optimizers/cplex/*.o
	rm -f optimizers/cplex/*.d
	
clean-scip:
	rm -f optimizers/cplex/*.o
	rm -f optimizers/cplex/*.d

clean-glucose3:
	$(MAKE) -C solvers/glucose3 clean

clean-glucose3x:
	$(MAKE) -C solvers/glucose3x clean

clean-cadical:
	$(MAKE) -C ../cadical clean
	cd ../cadical && ./configure

clean-maxpre:
	$(MAKE) -C ../maxpre2 clean
