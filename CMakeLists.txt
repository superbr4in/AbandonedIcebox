cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory("submodules/")

include_directories("modules/")

add_subdirectory("modules/")
add_subdirectory("test/")
