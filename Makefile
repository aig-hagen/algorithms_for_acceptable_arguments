SRCDIR    = src
BUILDDIR  = build
TARGET    = serial-solver

SRCEXT    = cpp
ALLSRCS   = $(wildcard $(SRCDIR)/*.$(SRCEXT))
SATSRCS   = $(wildcard $(SRCDIR)/*CryptoMiniSatSolver.$(SRCEXT))
SOURCES   = $(filter-out $(SATSRCS),  $(ALLSRCS))
OBJECTS   = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

CXX       = g++
CFLAGS    = -Wall -Wno-parentheses -Wno-sign-compare -std=c++11
COPTIMIZE = -O3
LFLAGS    = -Wall
IFLAGS    = -I include

#LDFLAGS = -lboost_thread-mt
LDFLAGS = -lpthread

CFLAGS   += $(COPTIMIZE)
CFLAGS   += -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS
CFLAGS   += -D CONE_OF_INFLUENCE
#CFLAGS   += -D DEBUG_MODE

#SAT_SOLVER = cryptominisat
SAT_SOLVER = external
#CMSAT      = lib/cryptominisat-5.8.0

ifeq ($(SAT_SOLVER), cryptominisat)
	CFLAGS  += -D SAT_CMSAT
	IFLAGS  += -I $(CMSAT)/build/include
	LFLAGS  += -Wl,-rpath,'$$ORIGIN/lib/cryptominisat-5.8.0/build/lib' -L $(CMSAT)/build/lib -lcryptominisat5
	OBJECTS += $(BUILDDIR)/CryptoMiniSatSolver.o
	CMSAT_BUILD = $(CMSAT)/build
else
	$(@echo No SAT solver specified. Configured for external sat solver)
endif

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@echo "$(CXX) $(OBJECTS) -o $(TARGET) $(LFLAGS)" $(LDFLAGS); $(CXX) $(OBJECTS) -o $(TARGET) $(LFLAGS) $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo "Compiling..."
	@mkdir -p $(BUILDDIR)
	@echo "$(CXX) $(CFLAGS) $(IFLAGS) -c -o $@ $<"; $(CXX) $(CFLAGS) $(IFLAGS) -c -o $@ $<

#.ONESHELL:
cmsat:
	@echo "Compiling CryptoMiniSat..."
	cd lib/cryptominisat-5.8.0 && \
	mkdir -p build && cd build && \
	cmake .. && \
	make

clean:
	@echo "Cleaning..."
	@echo "rm -rf $(BUILDDIR) $(TARGET)"; rm -rf $(BUILDDIR) $(TARGET)
