# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
TARGET_EXEC	:= solver

SAT_SOLVER	?= cryptominisat
ALGORITHM	?= IAQ

BUILD_DIR 	:= ./build
SRC_DIRS 	:= ./src
INC_DIRS 	:= ./include

ifeq ($(SAT_SOLVER), external)
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

CPPFLAGS	+= -D REL

##################################################################################################
###### CUSTOM ####################################################################################
##################################################################################################
CPPFLAGS += -Wall -Wno-parentheses -Wno-sign-compare

ifeq ($(SAT_SOLVER), cryptominisat)
	CPPFLAGS    += -D SAT_CMSAT
	LDFLAGS  	+= -lcryptominisat5
else ifeq ($(SAT_SOLVER), external)
	CPPFLAGS    += -D SAT_EXTERNAL
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

###################################################################################################

# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

#.ONESHELL:
cmsat:
	@echo "Compiling CryptoMiniSat..."
	cd lib/cryptominisat-5.11.4 && \
	mkdir -p build && cd build && \
	cmake .. && \
	make

.PHONY: clean
clean:
	@echo "Cleaning..."
	rm -r $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)