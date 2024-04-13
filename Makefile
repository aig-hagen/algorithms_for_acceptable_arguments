# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)

# Default values for the chosen SAT Solver and Algorithm
SOLVER		?= cryptominisat
ALGORITHM	?= IAQ

# Directories for the source of the SAT Solvers
GLUCOSE_DIR	= lib/glucose-syrup-4.1
CADICAL_DIR	= lib/cadical-1.9.5
CMSAT_DIR	= lib/cryptominisat-5.11.21
CGSS2_DIR	= lib/cgss2


# main directories of this project
BUILD_DIR 	:= ./build
SRC_DIRS 	:= ./src
INC_DIRS 	:= ./include


# include source of the chosen SAT Solver and set target executable name
ifeq ($(SOLVER), cryptominisat)
	INC_DIRS	+= ./$(CMSAT_DIR)/src
	TARGET_EXEC	:= solver_$(ALGORITHM)_cmsat
else ifeq ($(SOLVER), cadical)
	INC_DIRS	+= ./$(CADICAL_DIR)/src
	TARGET_EXEC	:= solver_$(ALGORITHM)_cadical
else ifeq ($(SOLVER), glucose)
	INC_DIRS	+= ./$(GLUCOSE_DIR)/
	TARGET_EXEC	:= solver_$(ALGORITHM)_glucose
else ifeq ($(SOLVER), external)
	INC_DIRS	+= ./lib/pstreams-1.0.3
	TARGET_EXEC	:= solver_$(ALGORITHM)_ext
else ifeq ($(SOLVER), cgss2)
	INC_DIRS	+= $(CGSS2_DIR)/src/
	TARGET_EXEC	:= solver_$(ALGORITHM)_cgss2
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
INC_FLAGS	:= $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS	:= $(INC_FLAGS) -MMD -MP

##################################################################################################
###### CUSTOM ####################################################################################
##################################################################################################
CPPFLAGS	+= -Wall -Wno-parentheses -Wno-sign-compare -std=c++20

#CPPFLAGS	+= -D PERF_ENC

ifeq ($(SOLVER), cryptominisat)
	CPPFLAGS    += -D SAT_CMSAT
	LDFLAGS  	+= -lcryptominisat5
else ifeq ($(SOLVER), cadical)
	CPPFLAGS	+= -D SAT_CADICAL
	LDFLAGS		+= $(CADICAL_DIR)/build/libcadical.a
else ifeq ($(SOLVER), glucose)
	CPPFLAGS	+= -D SAT_GLUCOSE
	LDFLAGS  	+= -L$(GLUCOSE_DIR)/build/dynamic/lib -lglucose
else ifeq ($(SOLVER), external)
	CPPFLAGS    += -D SAT_EXTERNAL
else ifeq ($(SOLVER), cgss2)
	CPPFLAGS	+= -D SAT_CGSS2
	LDFLAGS		+= $(CGSS2_DIR)/src/lib/libcgss2.a
endif

ifeq ($(ALGORITHM), IAQ)
	CPPFLAGS    += -D IAQ
else ifeq ($(ALGORITHM), EEE)
	CPPFLAGS    += -D EEE
else ifeq ($(ALGORITHM), SEE)
	CPPFLAGS    += -D SEE
else ifeq ($(ALGORITHM), SEEM)
	CPPFLAGS    += -D SEEM
else ifeq ($(ALGORITHM), FUDGE)
	CPPFLAGS    += -D FUDGE
else
	$(error No algorithm specified.)
endif

# debug
CXXFLAGS	+= -g3

OUTPUT_DIR	:= ./build/bin

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
	make && \
	sudo make install && \
	sudo ldconfig

cadical:
	@echo "Compiling CaDiCal..."
	cd $(CADICAL_DIR) && \
	./configure && make

glucose:
	@echo "Compiling Glucose..."
	cd $(GLUCOSE_DIR) && \
	make && \
	sudo make install && \
	sudo ldconfig

cgss2:
	@echo "Compiling CGSS2..."
	cd $(CGSS2_DIR)/cadical && ./configure && \
	cd ..  && mkdir -p src/solvers/lib && mkdir -p src/lib && \
	make && make lib

full:
	@$(MAKE) all SOLVER=external
	@$(MAKE) all SOLVER=cryptominisat
	@$(MAKE) all SOLVER=cadical
	@$(MAKE) all SOLVER=glucose
	@$(MAKE) all SOLVER=cgss2
	@$(MAKE) seem SOLVER=cgss2

all:
	@$(MAKE) iaq
	@$(MAKE) eee
	@$(MAKE) see
	@$(MAKE) fudge

iaq:
	@$(MAKE) clean-src
	@echo "Building solver for algorithm: IAQ..."
	@$(MAKE) ALGORITHM=IAQ
eee:
	$(MAKE) clean-src
	@echo "Building solver for algorithm: EEE..."
	@$(MAKE) ALGORITHM=EEE
see:
	$(MAKE) clean-src
	@echo "Building solver for algorithm: SEE..."
	@$(MAKE) ALGORITHM=SEE
seem:
	$(MAKE) clean-src
	@echo "Building solver for algorithm: SEEM..."
	@$(MAKE) ALGORITHM=SEEM
fudge:
	$(MAKE) clean-src
	@echo "Building solver for algorithm: Fudge..."
	@$(MAKE) ALGORITHM=FUDGE

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
