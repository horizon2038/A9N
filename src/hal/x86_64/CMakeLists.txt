# x86_64 HAL configuration

enable_language(ASM_NASM)

file(
    GLOB_RECURSE
    ARCH_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.s"
)

message(STATUS "hal sources : ${ARCH_SOURCES}")

set(ARCH_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# HAL(arch-specific) library configuration
# add_library(hal STATIC ${ARCH_SOURCES})
target_sources(
    hal
    PRIVATE
    ${ARCH_SOURCES}
)

target_include_directories(
    hal PUBLIC
    "${ARCH_INCLUDE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    "${CMAKE_SOURCE_DIR}/src/kernel/include"
)

target_link_libraries(
    hal
    PUBLIC
    liba9n
)

set_target_properties(hal PROPERTIES POSITION_INDEPENDENT_CODE OFF)
