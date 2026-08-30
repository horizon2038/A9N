# AArch64 bare-metal toolchain configuration (LLVM/Clang)

cmake_minimum_required(VERSION 3.29)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(LLVM_CONFIG_EXECUTABLE llvm-config)
if(NOT LLVM_CONFIG_EXECUTABLE)
    message(FATAL_ERROR "llvm-config is not found")
endif()

execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --bindir
    OUTPUT_VARIABLE LLVM_BINDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_C_COMPILER "${LLVM_BINDIR}/clang" CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "${LLVM_BINDIR}/clang++" CACHE FILEPATH "" FORCE)
set(CMAKE_ASM_COMPILER "${LLVM_BINDIR}/clang" CACHE FILEPATH "" FORCE)
find_program(LLD_EXECUTABLE NAMES ld.lld HINTS "${LLVM_BINDIR}" REQUIRED)
set(CMAKE_LINKER "${LLD_EXECUTABLE}" CACHE FILEPATH "" FORCE)
set(CMAKE_C_COMPILER_TARGET aarch64-none-elf)
set(CMAKE_CXX_COMPILER_TARGET aarch64-none-elf)
set(CMAKE_ASM_COMPILER_TARGET aarch64-none-elf)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

set(A9N_AARCH64_COMMON_FLAGS
    "--target=aarch64-none-elf -fuse-ld=lld -Wno-unused-command-line-argument -mcpu=cortex-a72 -mgeneral-regs-only -mno-outline-atomics -ffreestanding -nostdlib -fno-pic -fno-pie -fno-stack-protector -fno-threadsafe-statics -fdata-sections -ffunction-sections -flto=full"
)
set(CMAKE_C_FLAGS "${A9N_AARCH64_COMMON_FLAGS}" CACHE STRING "" FORCE)
set(
    CMAKE_CXX_FLAGS
    "${A9N_AARCH64_COMMON_FLAGS} -nostdinc++ -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fforce-emit-vtables"
    CACHE STRING "" FORCE
)
set(
    CMAKE_ASM_FLAGS_INIT
    "--target=aarch64-none-elf -Wno-unused-command-line-argument -mcpu=cortex-a72"
)
set(CMAKE_ASM_COMPILE_OBJECT
    "<CMAKE_ASM_COMPILER> <FLAGS> -c <SOURCE> -o <OBJECT>"
)

if(NOT DEFINED PLATFORM OR PLATFORM STREQUAL "")
    message(FATAL_ERROR "{PLATFORM} variable is required by the aarch64 toolchain")
endif()

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/src/hal/aarch64/platform/${PLATFORM}/kernel.ld")
set(MAP_FILE "${CMAKE_BINARY_DIR}/kernel.map")

set(CMAKE_C_LINK_EXECUTABLE
    "${LLD_EXECUTABLE} <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
)
set(CMAKE_CXX_LINK_EXECUTABLE
    "${LLD_EXECUTABLE} <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
)

add_link_options(
    -T "${LINKER_SCRIPT}"
    --static
    -nostdlib
    -Map "${MAP_FILE}"
    --gc-sections
)
