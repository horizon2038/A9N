SHELL=/bin/bash

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
SERVERDIR := ./src/servers

KERNEL_SRCS :=  $(shell find $(SRCDIR)/kernel -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
KERNEL_OBJS = $(addprefix $(BUILDDIR)/$(ARCH)/kernel/, $(addsuffix .o, $(basename $(KERNEL_SRCS:$(SRCDIR)/kernel/%=%))))
OBJS = $(KERNEL_OBJS)
DEPS = $(OBJS:.o=.d)

HAL_SRCS :=  $(shell find $(SRCDIR)/hal/$(ARCH) -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
HAL_OBJS = $(addprefix $(BUILDDIR)/$(ARCH)/hal/, $(addsuffix .o, $(basename $(HAL_SRCS:$(SRCDIR)/hal/%=%))))
OBJS += $(HAL_OBJS)
DEPS += $(HAL_OBJS:.o=.d)

LIB_SRCS :=  $(shell find $(SRCDIR)/liba9n -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
LIB_OBJS = $(addprefix $(BUILDDIR)/$(ARCH)/liba9n/, $(addsuffix .o, $(basename $(LIB_SRCS:$(SRCDIR)/liba9n/%=%))))
OBJS += $(LIB_OBJS)
DEPS += $(LIB_OBJS:.o=.d)

INCDIR = $(SRCDIR)/kernel/include
INCDIR += $(SRCDIR)/hal/include
INCDIR += $(SRCDIR)/hal/$(ARCH)/include
INCDIR += $(SRCDIR)/liba9n/include
INCFLAGS = $(addprefix -I,$(INCDIR))

CC := clang-16 
CXX := clang++-16
ASM := nasm
LD = ld.lld-16
CFLAGS = -g -O2 -Wall --target=$(ARCH)-elf -ffreestanding \
	-mno-red-zone -fno-pic -nostdlib -mcmodel=large -fomit-frame-pointer \
	-mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2 -fno-threadsafe-statics
# -flto

CXXFLAGS = -g -O2 -Wall --target=$(ARCH)-elf -ffreestanding \
	-mno-red-zone -fno-pic -nostdlib -mcmodel=large -fno-exceptions -fno-rtti -std=c++20 -fno-threadsafe-statics \
	-mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2
# -flto -fwhole-program-vtables -fforce-emit-vtables -fvirtual-function-elimination

CPPFLAGS = $(INCFLAGS) -MMD -MP
ASFLAGS = -f elf64

LDFLAGS = -T $(SRCDIR)/hal/$(ARCH)/kernel.ld -z norelro --static -nostdlib -Map build/kernel.map
LIBS = 

$(info $(make show-targets))

.PHONY: all show-targets kernel boot server test clean clean-kernel clean-boot clean-server clean-test 

all: kernel boot server test

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
$(BUILDDIR)/$(ARCH)/liba9n/$(TARGET): $(OBJS) $(LIBS)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(BUILDDIR)/$(ARCH)/liba9n/%.o: $(SRCDIR)/liba9n/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/liba9n/%.o: $(SRCDIR)/liba9n/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/liba9n/%.o: $(SRCDIR)/liba9n/%.s
	mkdir -p $(dir $@)
	$(ASM) $(ASFLAGS) $< -o $@ 

# boot
$(BUILDDIR)/$(ARCH)/boot/$(BOOT):
	ARCH=$(ARCH) LLVMDIR=$(LLVMDIR) $(SCRIPTSDIR)/build_a9nloader.sh
	mkdir -p $(BUILDDIR)/$(ARCH)/boot
	cp $(CHAINDIR)/$(ARCH)/edk2/build/$(A9NLOADER)/x64/DEBUG_CLANGPDB/X64/a9nloader.efi $@

server:
	$(MAKE) -C $(SERVERDIR)

test:
	$(MAKE) -C $(TESTDIR)
	$(TESTDIR)/build/test --gtest_color=yes --gtest_break_on_failure

clean:
	rm -f $(OBJS) $(DEPS) $(BUILDDIR)/$(ARCH)/kernel/$(TARGET)
	rm -f $(BUILDDIR)/$(ARCH)/boot/$(BOOT)
	make -C $(TESTDIR) clean

clean-kernel:
	rm -f $(OBJS) $(DEPS) $(BUILDDIR)/$(ARCH)/kernel/$(TARGET)

clean-boot:
	rm -f $(BUILDDIR)/$(ARCH)/boot/$(BOOT)

clean-server:
	make -C $(SERVERDIR) clean

clean-test:
	make -C $(TESTDIR) clean

-include $(DEPS)
