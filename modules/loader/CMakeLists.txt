# modules/loader/

cmake_minimum_required(VERSION 3.11)

file(GLOB SOURCE_FILES "source/*.cpp")
add_library(loader SHARED ${SOURCE_FILES})
