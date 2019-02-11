# test/

cmake_minimum_required(VERSION 3.11)

file(GLOB_RECURSE SOURCE_FILES "*.cpp")
add_executable(test ${SOURCE_FILES})

target_link_libraries(test PRIVATE catch2
    disassembler
    loader
)
