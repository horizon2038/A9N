# hardware independent tests

TARGET := test

TESTSRCDIR := ./src
SRCDIR := ../src
BUILDDIR := ./build
SCRIPTDIR := ../scripts

LIBDIR := ./library

INCDIR := ../src/kernel/include
INCDIR += ../src/hal/include
INCDIR += ../src/hal/test/include
INCDIR += ../src/liba9n/include
INCDIR += ./src/include
LIBINCDIR := ./library/include
INCDIR += $(LIBINCDIR)

INCFLAGS = $(addprefix -I,$(INCDIR))
LIBFLAGS = $(addprefix -L,$(LIBDIR))

TEST_SRCS := $(shell find $(TESTSRCDIR) -type f -name "*.cpp")
TEST_OBJS := $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(TEST_SRCS:$(TESTSRCDIR)/%=%))))

# SRCS := $(SRCDIR)/kernel/capability/capability_node.cpp $(SRCDIR)/kernel/utility/logger.cpp $(SRCDIR)/kernel/utility/print.cpp
SRCS := $(shell find $(SRCDIR) -type f -name "*.cpp" | grep -E -v -e "main.cpp" -e "*/hal/*")
OBJS := $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SRCS:$(SRCDIR)/%=%))))

DEPS = $(TEST_OBJS:.o=.d)
DEPS += $(OBJS:.o=.d)

$(info $(TEST_SRCS))
$(info $(SRCS))

CXX = clang++-16
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -stdlib=libc++ -pthread -flto -fstrict-vtable-pointers
# -fsanitize=address,undefined
CXXTESTFLAGS = -lgtest_main -lgtest -lgmock_main -lgmock
# -fsanitize=address,undefined

CPPFLAGS = $(INCFLAGS) -MMD -MP

LD := ld.lld
LDFLAGS := -lc++ -lc++abi -pthread -flto

.PHONY: all clean

all: executable

executable: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(TEST_OBJS) $(OBJS)
	mkdir -p $(dir $@)
	$(CXX) $^ -o $@ $(LDFLAGS) $(CXXTESTFLAGS) $(LIBFLAGS) 

$(BUILDDIR)/%.o: $(TESTSRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)/*

-include $(DEPS)


