SHELL=/usr/bin/bash

TARGET = kernel.elf
SRCDIR ?= ./kernel
BUILDDIR ?= ./build

# SRCS = $(shell find $(SRCDIR) -name "*.c" -or -name "*.cpp" -or -name "*.s")
SRCS = $(shell find $(SRCDIR) -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(OBJS))
OBJS := $(patsubst $(SRCDIR)/%.s,$(BUILDDIR)/%.o,$(OBJS))
# OBJS = $(patsubst $(SRCDIR)%,$(BUILDDIR)%,$(SRCS:.c=.o))
# OBJS := $(patsubst $(SRCDIR)%,$(BUILDDIR)%,$(OBJS:.cpp=.o))
# OBJS := $(patsubst $(SRCDIR)%,$(BUILDDIR)%,$(OBJS:.s=.o))
# OBJS = $(SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
# OBJS := $(OBJS:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
# OBJS := $(OBJS:$(SRCDIR/)%.s=$(BUILDDIR)/%.o)
DEPS = $(OBJS:.o=.d)

INCDIR = $(shell find $(SRCDIR) -type d)
INCFLAGS = $(addprefix -I,$(INCDIR))

CC := clang++ 
CXX := clang++
ASM := nasm
LD = ld.lld
ARCHITECTURE = x86_64
ARCHITECTURECFLAGS := x86_64-elf
CFLAGS = -O2 -Wall -g --target=$(ARCHITECTURECFLAGS) -ffreestanding -mno-red-zone
CXXFLAGS = -O2 -Wall -g --target=$(ARCHITECTURECFLAGS) -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -std=c++17
CPPFLAGS = $(INCFLAGS) -MMD -MP -I. -I./kernel/include -I./kernel/hal/include
ASFLAGS = -f elf64
LDFLAGS = --entry kernel_main -z norelro --image-base 0x100000 --static
LIBS = 

.PHONY: all build clean run

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(OBJS) $(LIBS)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.s
	mkdir -p $(dir $@)
	$(ASM) $(ASFLAGS) -o $@ $<

build: all

clean:
	rm -f $(BUILDDIR)/*.elf	
	rm -f $(BUILDDIR)/*.o
	rm -f $(BUILDDIR)/*.d

-include $(DEPS)
