cmake_minimum_required(VERSION 3.11)

file(GLOB SOURCE_FILES "source/*.cpp")
file(GLOB SOURCE_TEST_FILES "source/test/*.cpp")

add_library(disassembler SHARED ${SOURCE_FILES})
add_executable(disassembler-test ${SOURCE_TEST_FILES})

target_include_directories(disassembler PRIVATE "include/" INTERFACE "include/")

target_link_libraries(disassembler PRIVATE capstone)
target_link_libraries(disassembler-test PRIVATE catch2 disassembler)
