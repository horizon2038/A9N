# HAL interface library configuration
# add_library(hal PUBLIC)
# target_include_directories(
#     hal_interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include"
# )

# HAL (arch-specific) library configuration
# defined in CMakeLists.txt of root
# add_subdirectory("${HAL_ARCH_DIR}")

# add_library(hal_interface INTERFACE)

# target_include_directories(
#     hal_interface
#     INTERFACE
#     "${CMAKE_CURRENT_SOURCE_DIR}/include"
#     "${CMAKE_SOURCE_DIR}/src/kernel/include"
#     "${SMAKE_SOURCE_DIR}/src/hal/${ARCH}/include"
# )
# 
# target_link_libraries(
#     hal_interface
#     INTERFACE
#     liba9n
# )

# target_include_directories(
#     hal_interface
#     PUBLIC
# )

# add_subdirectory(${ARCH})

# target_link_libraries(hal INTERFACE hal_interface hal_arch)
# target_include_directories(hal INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(hal OBJECT)

target_include_directories(
    hal
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src/kernel/include"
    "${CMAKE_SOURCE_DIR}/src/hal/include"
    "${CMAKE_SOURCE_DIR}/src/hal/${arch}/include"
)

target_link_libraries(
    hal
    PUBLIC
    liba9n
)

add_subdirectory(${ARCH})

