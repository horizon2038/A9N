# hardware independent tests

SRCDIR := ./src
BUILDDIR := ./build
SCRIPTDIR := ../scripts

INCDIR := ../src/kernel/include
INCDIR += ../src/hal/include
INCDIR += ../src/library/include
INCFLAGS = $(addprefix -I,$(INCDIR))

TEST_SRCS := $(shell find $(SRCDIR) -type f -name "*.cpp")


TARGETS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(TEST_SRCS))

$(info $(TEST_SRCS))
$(info $(TARGETS))

CXX = clang++
CXXFLAGS = -Wall -Wextra -O2 -std=c++20
CXXTESTFLAGS = -lgtest_main -lgtest

.PHONY: all clean

all: $(TARGETS)
	mkdir -p $(dir $@)
	@$(SCRIPTDIR)/test_all.sh $(realpath $(TARGETS))

$(TARGETS): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $(CXXTESTFLAGS) $(INCFLAGS) $^ -o $@

clean:
	rm -f $(TARGETS)


