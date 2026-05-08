TARGET := init.elf
SRCDIR ?= ./test
BUILDDIR ?= ./build
ARCH = x86_64
export ARCH

SERVER_SRCS :=  $(shell find $(SRCDIR) -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
SERVER_OBJS = $(addprefix $(BUILDDIR)/$(ARCH)/, $(addsuffix .o, $(basename $(SERVER_SRCS:$(SRCDIR)/%=%))))
OBJS = $(SERVER_OBJS)
DEPS = $(OBJS:.o=.d)

$(warning SERVER_SRCS = $(SERVER_SRCS))
$(warning SERVER_OBJS = $(SERVER_OBJS))

# LIB_SRCS :=  $(shell find $(SRCDIR)/liba9n -type f \( -name "*.c" -or -name "*.cpp" -or -name "*.s" \))
# LIB_OBJS = $(addprefix $(BUILDDIR)/$(ARCH)/liba9n/, $(addsuffix .o, $(basename $(LIB_SRCS:$(SRCDIR)/liba9n/%=%))))
# OBJS += $(LIB_OBJS)
# DEPS += $(LIB_OBJS:.o=.d)

INCDIR = $(SRCDIR)/include
# INCDIR += $(SRCDIR)/liba9n/include
INCFLAGS = $(addprefix -I,$(INCDIR))

CC := clang-16 
CXX := clang++-16
ASM := nasm
LD = ld.lld-16
CFLAGS = -g -O2 -Wall --target=$(ARCH)-elf -ffreestanding \
	-nostdlib -mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2 -fno-threadsafe-statics
# -flto

CXXFLAGS = -g -O2 -Wall --target=$(ARCH)-elf -ffreestanding \
	-fno-pic -nostdlib -fno-exceptions -fno-rtti -std=c++20 -fno-threadsafe-statics

CPPFLAGS = $(INCFLAGS) -MMD -MP
ASFLAGS = -f elf64

LDFLAGS = -T ./arch/$(ARCH)/user.ld --static -nostdlib -Map build/init.map

.PHONY: all init clean

all: init

init: $(BUILDDIR)/$(ARCH)/$(TARGET)

# init
$(BUILDDIR)/$(ARCH)/$(TARGET): $(OBJS) $(LIBS)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(BUILDDIR)/$(ARCH)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(ARCH)/%.o: $(SRCDIR)/%.s
	mkdir -p $(dir $@)
	$(ASM) $(ASFLAGS) $< -o $@ 

clean:
	rm -f $(OBJS) $(DEPS) $(BUILDDIR)/$(ARCH)/$(TARGET)

-include $(DEPS)
