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

set_target_properties(kernel.elf PROPERTIES POSITION_INDEPENDENT_CODE OFF)
