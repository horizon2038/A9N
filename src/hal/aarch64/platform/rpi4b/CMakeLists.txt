# Raspberry Pi 4 Model B platform configuration for AArch64

file(
    GLOB_RECURSE
    PLATFORM_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.s"
)

target_sources(hal PRIVATE ${PLATFORM_SOURCES})
target_include_directories(hal PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

find_program(LLVM_OBJCOPY_EXECUTABLE NAMES llvm-objcopy HINTS "${LLVM_BINDIR}" REQUIRED)
set(A9N_AARCH64_KERNEL_IMAGE "${CMAKE_BINARY_DIR}/src/kernel/kernel.img")
add_custom_target(
    aarch64_kernel_image ALL
    COMMAND
        "${LLVM_OBJCOPY_EXECUTABLE}" -O binary "$<TARGET_FILE:kernel.elf>"
        "${A9N_AARCH64_KERNEL_IMAGE}"
    DEPENDS kernel.elf
    BYPRODUCTS "${A9N_AARCH64_KERNEL_IMAGE}"
    COMMENT "Generating Raspberry Pi 4 U-Boot-compatible AArch64 Image"
)
install(FILES "${A9N_AARCH64_KERNEL_IMAGE}" DESTINATION .)

message(STATUS "aarch64 platform : ${PLATFORM}")
message(STATUS "hal sources : ${ARCH_SOURCES};${PLATFORM_SOURCES}")
