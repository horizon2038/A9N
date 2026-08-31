# kernel build configuration

set(KERNEL_BINARY "kernel.elf")

file(
    GLOB_RECURSE
    KERNEL_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

message(STATUS "kernel sources : ${KERNEL_SOURCES}")

set(KERNEL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(${KERNEL_BINARY} ${KERNEL_SOURCES})

set(
    A9N_KERNEL_CONFIG_OPTIONS
    A9N_CONFIG_ENABLE_LOG_ID
    A9N_CONFIG_ENABLE_LOG_CORE_ID
    A9N_CONFIG_ENABLE_LOG_FAULT
    A9N_CONFIG_ENABLE_LOG_DOUBLE_FAULT
)
a9n_apply_target_config_options(${KERNEL_BINARY} A9N_KERNEL_CONFIG_OPTIONS)

install(
    TARGETS ${KERNEL_BINARY}
    RUNTIME DESTINATION .
)

target_include_directories(
    ${KERNEL_BINARY}
    PRIVATE
    "${KERNEL_INCLUDE_DIR}"
    "${CMAKE_SOURCE_DIR}/src/hal/include"
    "${CMAKE_SOURCE_DIR}/src/hal/${ARCH}/include"
)

target_link_libraries(
    ${KERNEL_BINARY}
    PRIVATE
    $<TARGET_OBJECTS:hal>
    liba9n
)

set_target_properties(${KERNEL_BINARY} PROPERTIES POSITION_INDEPENDENT_CODE OFF)

# add_compile_options(
#     $<$<CONFIG:Debug>:-g -O0>
#     $<$<CONFIG:Release>:-O3>
# )

add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG_BUILD>
)


set(KERNEL_MAJOR_VERSION 0)
set(KERNEL_MINOR_VERSION 2)
set(KERNEL_PATCH_VERSION 2)

set(KERNEL_VERSION_STAGE "smp")
string(TIMESTAMP KERNEL_VERSION_DATE "%Y%m%d")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(KERNEL_VERSION_BUILD_TYPE "Debug")
else()
    set(KERNEL_VERSION_BUILD_TYPE "Release")
endif()

string(
    REPLACE
    "_"
    "-"
    ARCH_VERSION_STRING
    "${ARCH}"
)

set(KERNEL_VERSION "${KERNEL_MAJOR_VERSION}.${KERNEL_MINOR_VERSION}.${KERNEL_PATCH_VERSION}-${KERNEL_VERSION_STAGE}+${ARCH_VERSION_STRING}-${KERNEL_VERSION_BUILD_TYPE}-${KERNEL_VERSION_DATE}")

target_compile_definitions(
    ${KERNEL_BINARY}
    PRIVATE
    KERNEL_VERSION="${KERNEL_VERSION}"
)
