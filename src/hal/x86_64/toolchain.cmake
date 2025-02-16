# x86_64 : toolchain configuration (clang version)

# CMake base settings
cmake_minimum_required(VERSION 3.29)

# system configuration
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
# set(CMAKE_C_COMPILER_TARGET x86_64-unknown-none-elf)

# clang global
set(CLANG_VERSION 19)

# C configuration
# set(CMAKE_C_COMPILER clang-${CLANG_VERSION})
set(CMAKE_C_COMPILER clang CACHE STRING "" FORCE)
set(
    CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} \
    --target=x86_64-elf \
    -mcmodel=large \
    -mno-red-zone \
    -fno-pic \
    -fno-pie \
    -fomit-frame-pointer \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-avx \
    -mno-avx2 \
    -fno-threadsafe-statics \
    -ffreestanding \
    -nostdlib \
    -fdata-sections \
    -ffunction-sections \
    -flto=full \
    -fmacro-prefix-map=/Users/horizon/Documents/Program/A9N/= \
    "
)

# C++ configuration
# set(CMAKE_CXX_COMPILER clang++-${CLANG_VERSION})
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "" FORCE)
set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
    --target=x86_64-elf \
    -mcmodel=large \
    -mno-red-zone \
    -fno-pic \
    -fno-pie \
    -fomit-frame-pointer \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-avx \
    -mno-avx2 \
    -fno-threadsafe-statics \
    -fno-exceptions \
    -fno-rtti \
    -ffreestanding \
    -nostdlib \
    -fdata-sections \
    -ffunction-sections \
    -flto=full \
    -fforce-emit-vtables \
    -fmacro-prefix-map=/Users/horizon/Documents/Program/A9N/= \
    "
)

add_compile_options(
    $<$<CONFIG:DEBUG>:-g>
    $<$<CONFIG:DEBUG>:-O0>
    $<$<CONFIG:RELEASE>:-O3>
    $<$<CONFIG:RELEASE>:-funroll-loops>
    $<$<CONFIG:RELEASE>:-ftree-vectorize>
    $<$<CONFIG:RELEASE>:-ftree-vectorizer-verbose>
)

message(STATUS "CXX Compiler Flags : ${CMAKE_CXX_FLAGS}")
# -flto -fwhole-program-vtables -fforce-emit-vtables -fvirtual-function-elimination

# Asm configuration
find_program(NASM_EXECUTABLE nasm)
if (NOT NASM_EXECUTABLE)
    message(FATAL_ERROR "assembler error : NASM does not exist")
endif()
set(CMAKE_ASM_NASM_COMPILER ${NASM_EXECUTABLE})
set(CMAKE_ASM_NASM_FLAGS "-f elf64 -O5")
set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS s nasm asm)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> ${CMAKE_ASM_NASM_FLAGS} -o <OBJECT> <SOURCE>")

message(STATUS "linker type : ${CMAKE_LINKER_TYPE}")
# linker configuration
set(CMAKE_LINKER_TYPE "lld_launcher")
# lld configuration
find_program(LLD_EXECUTABLE ld.lld)
if (NOT LLD_EXECUTABLE)
    message(FATAL_ERROR "linker error : LLD does not exist")
endif()

set(CMAKE_C_COMPILER_LINKER "${LLD_EXECUTABLE}")
set(CMAKE_CXX_COMPILER_LINKER "${LLD_EXECUTABLE}")
set(CMAKE_LINKER "${LLD_EXECUTABLE}")
set(CMAKE_LINKER_lld_launcher "${LLD_EXECUTABLE}")
set(CMAKE_C_USING_LINKER_lld_launcher "${LLD_EXECUTABLE}")
set(CMAKE_CXX_USING_LINKER_lld_launcher "${LLD_EXECUTABLE}")

set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> ${CMAKE_GNULD_IMAGE_VERSION} <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> ${CMAKE_GNULD_IMAGE_VERSION} <LINK_LIBRARIES>")

message(STATUS "CMAKE_CXX_LINK_EXECUTABLE : ${CMAKE_CXX_LINK_EXECUTABLE}")

set(LINKER_MODE TOOL)
set(CMAKE_C_USING_LINKER_MODE "${LINKER_MODE}")
set(CMAKE_CXX_USING_LINKER_MODE "${LINKER_MODE}")

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/src/hal/x86_64/kernel.ld")
set(MAP_FILE "${CMAKE_BINARY_DIR}/kernel.map")

message(STATUS "exe_linker_flags : ${LLD_EXECUTABLE}")
message(STATUS "linker launcher : ${CMAKE_C_LINKER_LAUNCHER}")

add_link_options(
    ${CMAKE_EXE_LINKER_FLAGS}
    -T "${LINKER_SCRIPT}"
    -z norelro
    --static
    -nostdlib
    -Map "${MAP_FILE}"
    --gc-sections
)

# !
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

message(STATUS "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")

# target_link_options()

# TODO: using mold
