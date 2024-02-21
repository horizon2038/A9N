# hardware independent tests

TARGET := test

SRCDIR := ./src
BUILDDIR := ./build
SCRIPTDIR := ../scripts

LIBDIR := ./library

INCDIR := ../src/kernel/include
INCDIR += ../src/hal/include
INCDIR += ../src/library/include
INCDIR += ./src/include
LIBINCDIR := ./library/include
INCDIR += $(LIBINCDIR)

INCFLAGS = $(addprefix -I,$(INCDIR))
LIBFLAGS = $(addprefix -L,$(LIBDIR))

TEST_SRCS := $(shell find $(SRCDIR) -type f -name "*.cpp")
TEST_OBJS := $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(TEST_SRCS:$(SRCDIR)/%=%))))

$(info $(TEST_SRCS))

CXX = clang++
CXXFLAGS = -Wall -Wextra -O2 -std=c++20
CXXTESTFLAGS = -pthread -lgtest_main -lgtest

.PHONY: all clean

all: executable

executable: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(TEST_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $(CPPFLAGS) $(INCFLAGS) $(LIBFLAGS) $(CXXTESTFLAGS) $^ -o $@
#	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)/*

-include $(DEPS)


