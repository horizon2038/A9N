SHELL=/usr/bin/bash

TARGET = kernel.elf
SRCDIR ?= ./kernel
BUILDDIR ?= ./build
ARCH = x86_64

# SRCS = $(shell find $(SRCDIR) -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
SRCS  = $(shell find $(SRCDIR) -path $(SRCDIR)/hal -prune -o -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \) -print)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(OBJS))
OBJS := $(patsubst $(SRCDIR)/%.s,$(BUILDDIR)/%.o,$(OBJS))
DEPS = $(OBJS:.o=.d)

ARCH_INCDIR = $(shell find $(SRCDIR)/hal/$(ARCH) -type d)
# HAL_SRCS =  $(shell find $(SRCDIR)/hal -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \) -not -path "$(SRCDIR)/hal/$(ARCH)/*")
# HAL_OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(HAL_SRCS))
# HAL_OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(HAL_OBJS))
# HAL_OBJS := $(patsubst $(SRCDIR)/%.s,$(BUILDDIR)/%.o,$(HAL_OBJS))
HAL_SRCS :=  $(shell find $(SRCDIR)/hal -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
HAL_OBJS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(HAL_SRCS:$(SRCDIR)/%=%))))
OBJS += $(HAL_OBJS)
DEPS += $(HAL_OBJS:.o=.d)

INCDIR = $(shell find $(SRCDIR) -type d)
INCDIR += $(ARCH_INCDIR)
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

# $(BUILDDIR)/hal/%.o: $(SRCDIR)/hal/%.c
# 	mkdir -p $(dir $@)
# 	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
# 
# $(BUILDDIR)/hal/%.o: $(SRCDIR)/hal/%.cpp
# 	mkdir -p $(dir $@)
# 	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
# 	
# $(BUILDDIR)/hal/%.o: $(SRCDIR)/hal/%.s
# 	mkdir -p $(dir $@)
# 	$(ASM) $(ASFLAGS) -o $@ $<

build: all

clean:
	rm -f $(BUILDDIR)/*.elf	
	rm -f $(BUILDDIR)/*.o
	rm -f $(BUILDDIR)/*.d

-include $(DEPS)
