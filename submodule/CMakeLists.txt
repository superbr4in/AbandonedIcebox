cmake_minimum_required(VERSION 3.15)

set(BUILD_DOCUMENTATION   OFF CACHE BOOL "")
set(BUILD_DOTNET_BINDINGS OFF CACHE BOOL "")
set(BUILD_JAVA_BINDINGS   OFF CACHE BOOL "")
set(BUILD_LIBZ3_SHARED    ON  CACHE BOOL "")
set(BUILD_PYTHON_BINDINGS OFF CACHE BOOL "")

set(ENABLE_EXAMPLE_TARGETS       OFF CACHE BOOL "")
set(ENABLE_TRACING_FOR_NON_DEBUG OFF CACHE BOOL "")

add_subdirectory(z3)

# ./autogen.sh
# ./configure
# make
add_library(openreil
    STATIC
    IMPORTED GLOBAL)
set_target_properties(openreil PROPERTIES
    IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/openreil/libopenreil/src/libopenreil.a)
target_include_directories(openreil
    INTERFACE
        openreil/libopenreil/include)

add_library(z3
    ALIAS
        libz3)
target_include_directories(libz3
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/z3/src/api>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/z3/src/api/c++>)