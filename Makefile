SHELL=/usr/bin/bash

TARGET = kernel.elf
BOOT = BOOTX64.EFI
SRCDIR ?= ./src/kernel
BUILDDIR ?= ./build
ARCH = x86_64
export ARCH
CHAINDIR = ./chain
A9NLOADER = a9nloaderPkg
SCRIPTSDIR = ./scripts
LLVMDIR = /usr/local/opt/llvm/bin

SRCS  = $(shell find $(SRCDIR) -path $(SRCDIR)/hal -prune -o -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \) -print)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/$(ARCH)/kernel/%.o,$(SRCS))
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/$(ARCH)/kernel/%.o,$(OBJS))
OBJS := $(patsubst $(SRCDIR)/%.s,$(BUILDDIR)/$(ARCH)/kernel/%.o,$(OBJS))
DEPS = $(OBJS:.o=.d)

$(warning SRCS: $(SRCS))
$(warning OBJS: $(OBJS))

ARCH_INCDIR = $(shell find $(SRCDIR)/hal/$(ARCH) -type d)
HAL_SRCS :=  $(shell find $(SRCDIR)/hal -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
HAL_OBJS = $(addprefix $(BUILDDIR)/$(ARCH)/kernel/, $(addsuffix .o, $(basename $(HAL_SRCS:$(SRCDIR)/%=%))))
OBJS += $(HAL_OBJS)
DEPS += $(HAL_OBJS:.o=.d)

INCDIR = $(shell find $(SRCDIR) -type d)
INCDIR += $(ARCH_INCDIR)
INCFLAGS = $(addprefix -I,$(INCDIR))

CC := clang 
CXX := clang++
ASM := nasm
LD = ld.lld
CFLAGS = -O2 -Wall -g --target=$(ARCH)-elf -ffreestanding -mno-red-zone
CXXFLAGS = -O2 -Wall -g --target=$(ARCH)-elf -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -std=c++17
CPPFLAGS = $(INCFLAGS) -MMD -MP -I. -I$(SRCDIR)/include -I$(SRCDIR)/hal/include
ASFLAGS = -f elf64
LDFLAGS = --entry kernel_main -z norelro --image-base 0x100000 --static
LIBS = 

.PHONY: all kernel boot clean

all: kernel boot

kernel: $(BUILDDIR)/$(ARCH)/kernel/$(TARGET)

boot: $(BUILDDIR)/$(ARCH)/boot/$(BOOT)

$(BUILDDIR)/$(ARCH)/kernel/$(TARGET): $(OBJS) $(LIBS)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(BUILDDIR)/$(ARCH)/kernel/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/kernel/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/kernel/%.o: $(SRCDIR)/%.s
	mkdir -p $(dir $@)
	$(ASM) $(ASFLAGS) -o $@ $<

$(BUILDDIR)/$(ARCH)/boot/$(BOOT):
	ARCH=$(ARCH) LLVMDIR=$(LLVMDIR) $(SCRIPTSDIR)/build_a9nloader.sh
	cp $(CHAINDIR)/$(ARCH)/edk2/build/$(A9NLOADER)/x64/DEBUG_GCC5/X64/a9nloader.efi $@

clean:
	rm -f $(OBJS) $(DEPS) $(BUILDDIR)/$(ARCH)/kernel/$(TARGET)
	rm -f $(BUILDDIR)/$(ARCH)/boot/$(BOOT)

-include $(DEPS)
