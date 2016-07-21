# Created by aszdrick graf <aszdrick@gmail.com>
# Compiler
CXX       :=gcc
LDLIBS    :=-lstdc++ -lm `pkg-config --libs gtk+-3.0` -lgtest -pthread
LDFLAGS   :=
CXXFLAGS  :=`pkg-config --cflags gtk+-3.0` -O3 -std=c++11 -Wall
# Source directory
SRCDIR    :=src
# Headers directory
HDRDIR    :=include
# Build directory
BUILDIR   :=build
# Binaries directory
BINDIR    :=bin
# Tests directory
TESTDIR   :=tests
# Main file
MAIN      :=main
#Include flag
INCLUDE   :=-I$(HDRDIR) -I$(HDRDIR)/shapes
# Sources
SRC       :=$(shell find $(SRCDIR) -name '*.cpp')
# Test(s) source(s)
TESTSRC   :=$(shell find $(TESTDIR) -name '*.cpp')
# Dependencies
DEP       :=$(SRC:.cpp=.d) $(TESTSRC:.cpp=.d)
# Objects
OBJ       :=$(patsubst $(SRCDIR)/%.cpp,$(BUILDIR)/%.o,$(SRC))
# Pure objects, without main
PUREOBJ   :=$(filter-out $(BUILDIR)/$(MAIN).o,$(OBJ))
# Test(s) object(s)
TESTOBJ     :=$(patsubst %.cpp,$(BUILDIR)/%.o,$(TESTSRC))
# Program executable
EXEC      :=$(BINDIR)/exec
# Test(s) executable(s)
TESTS     :=$(patsubst $(TESTDIR)/%.cpp,$(BINDIR)/%,$(TESTSRC))

.PHONY: all makedir tests clean clean_all

all: makedir $(EXEC)

$(EXEC): $(OBJ)
	@echo "[linking] $@"
	@$(CXX) $(OBJ) -o $@ $(LDLIBS) $(LDFLAGS)

$(BUILDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "[  $(CXX)  ] $< -> .o"
	@mkdir -p $(BUILDIR)/$(*D)
	@$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

makedir: | $(BUILDIR) $(BINDIR)

$(BINDIR) $(BUILDIR):
	@echo "[ mkdir ] Creating directory '$@'"
	@mkdir -p $@

# For each .cpp file, creates a .d file with all dependencies of .cpp,
# including .d as target (to ensure updated dependencies, in case of
# adding a new include or nested include)
$(SRCDIR)/%.d: $(SRCDIR)/%.cpp
	@echo "[makedep] $< -> .d"
	@$(CXX) -MM -MP -MT "$(BUILDIR)/$*.o $@" -MF "$@" $< $(INCLUDE) $(CXXFLAGS)

tests: makedir $(TESTS)

$(TESTS): $(PUREOBJ) $(TESTOBJ)
	@echo "[linking] $@"
	@$(CXX) $(PUREOBJ) $(BUILDIR)/$(TESTDIR)/$(@F).o -o $@ $(LDLIBS) $(LDFLAGS)

$(BUILDIR)/%.o: %.cpp
	@echo "[  $(CXX)  ] $< -> .o"
	@mkdir -p $(BUILDIR)/$(*D)
	@$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(TESTDIR)/%.d: $(TESTDIR)/%.cpp
	@echo "[makedep] $< -> .d"
	@$(CXX) -MM -MP -MT "$(BUILDIR)/$(TESTDIR)$*.o $@" -MF "$@" $< $(INCLUDE) $(CXXFLAGS)

# Only remove object files
clean:
	@$(RM) -r $(BUILDIR)

# Remove object, binary and dependency files
clean_all:
	@$(RM) -r $(BUILDIR)
	@$(RM) -r $(BINDIR)
	@$(RM) -r $(DEP)

# Do not include list of dependencies if make was called with target clean_all
ifneq ($(MAKECMDGOALS), clean_all)
-include $(DEP)
endif
