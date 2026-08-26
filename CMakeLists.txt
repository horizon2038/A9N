# CMake base configuration
cmake_minimum_required(VERSION 3.29)

# project base configuration
project(
    A9N
    VERSION 0.1.8
    DESCRIPTION "A9N Microkernel"
    LANGUAGES C CXX
)

# .
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
set(BUILD_SHARED_LIBS OFF)

# C++ base configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS OFF)

function(a9n_apply_global_config_options options_variable)
    foreach(option_name IN LISTS ${options_variable})
        option(${option_name} "Enable ${option_name}" OFF)
        if(${option_name})
            add_compile_definitions(${option_name})
        endif()
    endforeach()
endfunction()

function(a9n_apply_target_config_options target options_variable)
    foreach(option_name IN LISTS ${options_variable})
        option(${option_name} "Enable ${option_name}" OFF)
        if(${option_name})
            target_compile_definitions(${target} PRIVATE ${option_name})
        endif()
    endforeach()
endfunction()

set(
    A9N_CONFIG_OPTIONS
    A9N_CONFIG_ENABLE_SMP
    A9N_CONFIG_ENABLE_HYPERVISOR
)
a9n_apply_global_config_options(A9N_CONFIG_OPTIONS)

# C++ global configuration
 set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
message("LTO : ${CMAKE_INTERPROCEDURAL_OPTIMIZATION}")

# arch configuration
if(NOT DEFINED ARCH)
    message(FATAL_ERROR "{ARCH} variable is not set")
endif()
set(HAL_ARCH_DIR "${CMAKE_SOURCE_DIR}/src/hal/${ARCH}")
if (NOT EXISTS "${HAL_ARCH_DIR}")
    message(FATAL_ERROR "the source directory for the specified ${ARCH} does not exist")
endif()

add_subdirectory(src)

add_compile_options(
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)
