cmake_minimum_required(VERSION 3.11)

file(GLOB SOURCE_FILES "source/*.cpp")

add_library(disassembler SHARED ${SOURCE_FILES})

target_include_directories(disassembler PRIVATE "include/" INTERFACE "include/")

target_link_libraries(disassembler PRIVATE capstone)
