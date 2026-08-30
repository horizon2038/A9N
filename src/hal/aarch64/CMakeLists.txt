# AArch64 HAL configuration

if(NOT DEFINED PLATFORM OR PLATFORM STREQUAL "")
    message(FATAL_ERROR "{PLATFORM} variable is not set for aarch64")
endif()

set(HAL_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platform/${PLATFORM}")
if(NOT EXISTS "${HAL_PLATFORM_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "the source directory for aarch64 platform ${PLATFORM} does not exist")
endif()

enable_language(ASM)

file(
    GLOB_RECURSE
    ARCH_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/arch/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/arch/*.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/boot/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interrupt/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interrupt/*.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/io/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/memory/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/process/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/virtualization/*.cpp"
)

target_sources(hal PRIVATE ${ARCH_SOURCES})

target_include_directories(
    hal
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${HAL_PLATFORM_DIR}/include"
)

set(A9N_HAL_CONFIG_OPTIONS)
a9n_apply_target_config_options(hal A9N_HAL_CONFIG_OPTIONS)
set_target_properties(hal PROPERTIES POSITION_INDEPENDENT_CODE OFF)

add_subdirectory("platform/${PLATFORM}")
