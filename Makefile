# SHELL=/usr/bin/bash

TARGET = kernel.elf
BOOT = BOOTX64.EFI
SRCDIR ?= ./src
BUILDDIR ?= ./build
ARCH = x86_64
export ARCH
CHAINDIR = ./chain
A9NLOADER = a9nloaderPkg
SCRIPTSDIR = ./scripts
TESTDIR := ./test
LLVMDIR = /usr/local/opt/llvm/bin

KERNEL_SRCS :=  $(shell find $(SRCDIR)/kernel -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
KERNEL_OBJS = $(addprefix $(BUILDDIR)/$(ARCH)/kernel/, $(addsuffix .o, $(basename $(KERNEL_SRCS:$(SRCDIR)/kernel/%=%))))
OBJS = $(KERNEL_OBJS)
DEPS = $(OBJS:.o=.d)

HAL_SRCS :=  $(shell find $(SRCDIR)/hal/$(ARCH) -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
HAL_OBJS = $(addprefix $(BUILDDIR)/$(ARCH)/hal/, $(addsuffix .o, $(basename $(HAL_SRCS:$(SRCDIR)/hal/%=%))))
OBJS += $(HAL_OBJS)
DEPS += $(HAL_OBJS:.o=.d)

LIB_SRCS :=  $(shell find $(SRCDIR)/library -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
LIB_OBJS = $(addprefix $(BUILDDIR)/$(ARCH)/library/, $(addsuffix .o, $(basename $(LIB_SRCS:$(SRCDIR)/library/%=%))))
OBJS += $(LIB_OBJS)
DEPS += $(LIB_OBJS:.o=.d)

INCDIR = $(SRCDIR)/kernel/include
INCDIR += $(SRCDIR)/hal/include
INCDIR += $(SRCDIR)/hal/$(ARCH)/include
INCDIR += $(SRCDIR)/library/include
INCFLAGS = $(addprefix -I,$(INCDIR))

CC := clang 
CXX := clang++
ASM := nasm
LD = ld.lld
CFLAGS = -g -O2 -Wall --target=$(ARCH)-elf -ffreestanding -mno-red-zone -fno-pic -nostdlib -mcmodel=large -masm=intel -fomit-frame-pointer -mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2
CXXFLAGS = -g -O2 -Wall --target=$(ARCH)-elf -ffreestanding -mno-red-zone -fno-pic -nostdlib -mcmodel=large -fno-exceptions -fno-rtti -std=c++20 -masm=intel
CPPFLAGS = $(INCFLAGS) -MMD -MP
ASFLAGS = -f elf64

LDFLAGS = -T $(SRCDIR)/hal/$(ARCH)/kernel.ld -z norelro --static -nostdlib -Map build/kernel.map
LIBS = 

$(info $(make show-targets))

.PHONY: all show-targets kernel boot test clean clean-kernel clean-boot clean-test

all: kernel boot test

show-targets:
	@echo -e "\e[44mTOOLS\e[0m :"
	@echo -e "\e[4G C \e[15G : $(CC)"
	@echo -e "\e[4G C++ \e[15G : $(CXX)"
	@echo -e "\e[4G ASM \e[15G : $(ASM)"
	@echo -e "\e[4G LINKER \e[15G : $(LD)"
	@echo ""

	@echo -e "\e[44mOUTPUT\e[0m :"
	@echo -e "\e[4G KERNEL \e[15G : $(TARGET)"
	@echo -e "\e[4G BOOT \e[15G : $(BOOT)"
	@echo ""

	@echo -e "\e[44mKERNEL\e[0m :"
	@$(foreach target, $(KERNEL_SRCS), echo -e "\e[4G ${target}";)
	@echo ""
	@$(foreach target, $(KERNEL_OBJS), echo -e "\e[4G $(target)";)
	@echo -e ""

	@echo -e "\e[44mHAL\e[0m :"
	@$(foreach target, $(HAL_SRCS), echo -e "\e[4G $(target)";)
	@echo ""
	@$(foreach target, $(HAL_OBJS), echo -e "\e[4G $(target)";)
	@echo -e ""

	@echo -e "\e[44mLIBRARY\e[0m :"
	@$(foreach target, $(LIB_SRCS), echo -e "\e[4G $(target)";)
	@echo ""
	@$(foreach target, $(LIB_OBJS), echo -e "\e[4G $(target)";)
	@echo -e ""

kernel: $(BUILDDIR)/$(ARCH)/kernel/$(TARGET)
	@make show-targets

boot: $(BUILDDIR)/$(ARCH)/boot/$(BOOT)

# kernel
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

# hal
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

# library
$(BUILDDIR)/$(ARCH)/library/$(TARGET): $(OBJS) $(LIBS)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(BUILDDIR)/$(ARCH)/library/%.o: $(SRCDIR)/library/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/library/%.o: $(SRCDIR)/library/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/library/%.o: $(SRCDIR)/library/%.s
	mkdir -p $(dir $@)
	$(ASM) $(ASFLAGS) $< -o $@ 

# boot
$(BUILDDIR)/$(ARCH)/boot/$(BOOT):
	ARCH=$(ARCH) LLVMDIR=$(LLVMDIR) $(SCRIPTSDIR)/build_a9nloader.sh
	mkdir -p $(BUILDDIR)/$(ARCH)/boot
	cp $(CHAINDIR)/$(ARCH)/edk2/build/$(A9NLOADER)/x64/DEBUG_GCC5/X64/a9nloader.efi $@

test:
	make -C $(TESTDIR)
	$(TESTDIR)/build/test

clean:
	rm -f $(OBJS) $(DEPS) $(BUILDDIR)/$(ARCH)/kernel/$(TARGET)
	rm -f $(BUILDDIR)/$(ARCH)/boot/$(BOOT)
	make -C $(TESTDIR) clean

clean-kernel:
	rm -f $(OBJS) $(DEPS) $(BUILDDIR)/$(ARCH)/kernel/$(TARGET)

clean-boot:
	rm -f $(BUILDDIR)/$(ARCH)/boot/$(BOOT)

clean-test:
	make -C $(TESTDIR) clean

-include $(DEPS)
