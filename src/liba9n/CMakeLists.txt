# liba9n configuration

file(
    GLOB_RECURSE
    LIBA9N_SOURCES
    CONFIGURE_DEPENDES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

set(LIBA9N_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_library(liba9n STATIC ${LIBA9N_SOURCES})

set(A9N_LIBA9N_CONFIG_OPTIONS)
a9n_apply_target_config_options(liba9n A9N_LIBA9N_CONFIG_OPTIONS)

target_include_directories(
    liba9n
    PUBLIC
    "${LIBA9N_INCLUDE_DIR}"
)
set_target_properties(liba9n PROPERTIES POSITION_INDEPENDENT_CODE OFF)

message(STATUS ${LIBA9N_SOURCES})
message(STATUS ${LIBA9N_INCLUDE_DIR})
