SHELL=/usr/bin/bash

TARGET = kernel.elf
BOOT = BOOTX64.EFI
SRCDIR ?= ./src
BUILDDIR ?= ./build
ARCH = x86_64
export ARCH
CHAINDIR = ./chain
A9NLOADER = a9nloaderPkg
SCRIPTSDIR = ./scripts
LLVMDIR = /usr/local/opt/llvm/bin

SRCS  = $(shell find $(SRCDIR)/kernel -path $(SRCDIR)/hal/$(ARCH) -prune -o -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \) -print)
OBJS = $(patsubst $(SRCDIR)/kernel/%.c,$(BUILDDIR)/$(ARCH)/kernel/%.o,$(SRCS))
OBJS := $(patsubst $(SRCDIR)/kernel/%.cpp,$(BUILDDIR)/$(ARCH)/kernel/%.o,$(OBJS))
OBJS := $(patsubst $(SRCDIR)/kernel/%.s,$(BUILDDIR)/$(ARCH)/kernel/%.o,$(OBJS))
DEPS = $(OBJS:.o=.d)

ARCH_INCDIR = $(shell find $(SRCDIR)/hal/$(ARCH) -type d)
HAL_SRCS :=  $(shell find $(SRCDIR)/hal/$(ARCH) -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
HAL_OBJS = $(addprefix $(BUILDDIR)/$(ARCH)/hal/, $(addsuffix .o, $(basename $(HAL_SRCS:$(SRCDIR)/hal/%=%))))
OBJS += $(HAL_OBJS)
DEPS += $(HAL_OBJS:.o=.d)

INCDIR = $(shell find $(SRCDIR)/kernel -type d)
INCDIR += $(ARCH_INCDIR)
INCFLAGS = $(addprefix -I,$(INCDIR))

$(warning SRCS: $(SRCS))
$(warning OBJS: $(OBJS))
$(warning HAL_SRCS: $(HAL_SRCS))
$(warning HAL_OBJS: $(HAL_OBJS))

CC := clang 
CXX := clang++
ASM := nasm
LD = ld.lld
CFLAGS = -g -O2 -Wall --target=$(ARCH)-elf -ffreestanding -mno-red-zone -no-pie -fno-pic -nostdlib -mcmodel=large -masm=intel -fomit-frame-pointer -mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2
CXXFLAGS = -g -O2 -Wall --target=$(ARCH)-elf -ffreestanding -mno-red-zone -no-pie -fno-pic -nostdlib -mcmodel=large -fno-exceptions -fno-rtti -std=c++17 -masm=intel
CPPFLAGS = $(INCFLAGS) -MMD -MP -I. -I$(SRCDIR)/kernel/include -I$(SRCDIR)/hal/include
ASFLAGS = -f elf64

# without linker-script (lower-half kernel)
# LDFLAGS = --entry kernel_entry -z norelro --image-base 0x100000 --static

# with linker-script (higher-half kernel)
LDFLAGS = -T $(SRCDIR)/hal/$(ARCH)/kernel.ld -z norelro --static -no-pie -nostdlib -Map build/kernel.map
LIBS = 

.PHONY: all kernel boot clean

all: kernel boot

kernel: $(BUILDDIR)/$(ARCH)/kernel/$(TARGET)

boot: $(BUILDDIR)/$(ARCH)/boot/$(BOOT)

$(BUILDDIR)/$(ARCH)/kernel/$(TARGET): $(OBJS) $(LIBS)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(BUILDDIR)/$(ARCH)/kernel/%.o: $(SRCDIR)/kernel/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/kernel/%.o: $(SRCDIR)/kernel/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/kernel/%.o: $(SRCDIR)/kernel/%.s
	mkdir -p $(dir $@)
	$(ASM) $(ASFLAGS) $< -o $@ 

$(BUILDDIR)/$(ARCH)/hal/$(TARGET): $(OBJS) $(LIBS)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(BUILDDIR)/$(ARCH)/hal/%.o: $(SRCDIR)/hal/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/hal/%.o: $(SRCDIR)/hal/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/hal/%.o: $(SRCDIR)/hal/%.s
	mkdir -p $(dir $@)
	$(ASM) $(ASFLAGS) $< -o $@ 

$(BUILDDIR)/$(ARCH)/boot/$(BOOT):
	ARCH=$(ARCH) LLVMDIR=$(LLVMDIR) $(SCRIPTSDIR)/build_a9nloader.sh
	cp $(CHAINDIR)/$(ARCH)/edk2/build/$(A9NLOADER)/x64/DEBUG_GCC5/X64/a9nloader.efi $@

clean:
	rm -f $(OBJS) $(DEPS) $(BUILDDIR)/$(ARCH)/kernel/$(TARGET)
	rm -f $(BUILDDIR)/$(ARCH)/boot/$(BOOT)

-include $(DEPS)
