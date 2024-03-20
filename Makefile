# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
TARGET_EXEC	:= solver

#shell = /bin/sh

SAT_SOLVER	?= cryptominisat
ALGORITHM	?= IAQ

GLUCOSE_DIR	= lib/glucose-4.2.1
CADICAL_DIR	= lib/cadical-1.9.5
CMSAT_DIR	= lib/cryptominisat-5.11.21

BUILD_DIR 	:= ./build
SRC_DIRS 	:= ./src
INC_DIRS 	:= ./include


ifeq ($(SAT_SOLVER), cryptominisat)
	INC_DIRS	+= ./$(CMSAT_DIR)/src
else ifeq ($(SAT_SOLVER), cadical)
	INC_DIRS	+= ./$(CADICAL_DIR)/src
else ifeq ($(SAT_SOLVER), glucose)
	INC_DIRS	+= ./$(GLUCOSE_DIR)/
else ifeq ($(SAT_SOLVER), evalmaxsat)
	INC_DIRS	+= ./lib/EvalMaxSAT/
	INC_DIRS	+= ./lib/EvalMaxSAT/lib/MaLib/src
#	INC_DIRS	+= ./lib/EvalMaxSAT/lib/cadical/src
else ifeq ($(SAT_SOLVER), external)
	INC_DIRS	+= ./lib/pstreams-1.0.3
endif

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise, but we want to send the * directly to the find command.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')

# Prepends BUILD_DIR and appends .o to every src file
# As an example, ./your_dir/hello.cpp turns into ./build/./your_dir/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP

##################################################################################################
###### CUSTOM ####################################################################################
##################################################################################################
CPPFLAGS += -Wall -Wno-parentheses -Wno-sign-compare -std=c++20

ifeq ($(SAT_SOLVER), cryptominisat)
	CPPFLAGS    += -D SAT_CMSAT
	LDFLAGS  	+= -lcryptominisat5
else ifeq ($(SAT_SOLVER), cadical)
	CPPFLAGS	+= -D SAT_CADICAL
	LDFLAGS		+= $(CADICAL_DIR)/build/libcadical.a
else ifeq ($(SAT_SOLVER), glucose)
	CPPFLAGS	+= -D SAT_GLUCOSE -D INCREMENTAL
	LDFLAGS  	+= $(GLUCOSE_DIR)/libglucose.a -lz
#	LDFLAGS  += -L$(GLUCOSE_DIR)/build/dynamic/lib -lglucose    ????
else ifeq ($(SAT_SOLVER), external)
	CPPFLAGS    += -D SAT_EXTERNAL
else ifeq ($(SAT_SOLVER), evalmaxsat)
	LDFLAGS		+= ./lib/EvalMaxSAT/build/lib/EvalMaxSAT/libEvalMaxSAT.a
	LDFLAGS		+= ./lib/EvalMaxSAT/build/lib/cadical/libcadical.a
	LDFLAGS		+= ./lib/EvalMaxSAT/build/lib/MaLib/libMaLib.a
	LDFLAGS		+= -lz
	CPPFLAGS	+= -D SAT_EVALMAXSAT -D NDEBUG
endif

ifeq ($(ALGORITHM), IAQ)
	CPPFLAGS    += -D IAQ
	OUTPUT_DIR	:= ./build/bin/iaq
else ifeq ($(ALGORITHM), EEE)
	CPPFLAGS    += -D EEE
	OUTPUT_DIR	:= ./build/bin/eee
else ifeq ($(ALGORITHM), SEE)
	CPPFLAGS    += -D SEE
	OUTPUT_DIR	:= ./build/bin/see
else ifeq ($(ALGORITHM), SEEM)
	CPPFLAGS    += -D SEEM
	OUTPUT_DIR	:= ./build/bin/seem
else ifeq ($(ALGORITHM), FUDGE)
	CPPFLAGS    += -D FUDGE
	OUTPUT_DIR	:= ./build/bin/fudge
else
	$(error No algorithm specified.)
endif

# debug
CXXFLAGS	+= -g3

###################################################################################################

# The final build step.
$(OUTPUT_DIR)/$(TARGET_EXEC): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

#.ONESHELL:
cmsat:
	@echo "Compiling CryptoMiniSat..."
	cd $(CMSAT_DIR) && \
	mkdir -p build && cd build && \
	cmake .. && \
	make

cadical:
	@echo "Compiling CaDiCal..."
	cd $(CADICAL_DIR) && \
	./configure && make

glucose:
	@echo "Compiling Glucose..."
	cd $(GLUCOSE_DIR) && \
	mkdir -p build && cd build && \
	cmake .. && \
	make glucose-syrup

all:
	@echo "Building solver for algorithm: IAQ..."
	$(MAKE) iaq
	$(MAKE) eee
	$(MAKE) see

iaq:
	$(MAKE) clean-src
	@echo "Building solver for algorithm: IAQ..."
	$(MAKE) ALGORITHM=IAQ
eee:
	$(MAKE) clean-src
	@echo "Building solver for algorithm: EEE..."
	$(MAKE) ALGORITHM=EEE
see:
	$(MAKE) clean-src
	@echo "Building solver for algorithm: SEE..."
	$(MAKE) ALGORITHM=SEE
seem:
	$(MAKE) clean-src
	@echo "Building solver for algorithm: SEEM..."
	$(MAKE) ALGORITHM=SEEM
fudge:
	$(MAKE) clean-src
	@echo "Building solver for algorithm: Fudge..."
	$(MAKE) ALGORITHM=FUDGE

clean-src:
	@if [ -d "build/src" ]; then \
		rm -rf build/src/*; \
		echo "Cleaned build/src directory."; \
	else \
		echo "build/src directory does not exist. Nothing to clean."; \
	fi

clean:
	@echo "Cleaning..."
	rm -r $(BUILD_DIR)

.PHONY: all clean clean-src

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
