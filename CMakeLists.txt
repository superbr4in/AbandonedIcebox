cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)

set(CAPSTONE_ARM_SUPPORT        OFF CACHE BOOL "Capstone: ARM support")
set(CAPSTONE_ARM64_SUPPORT      OFF CACHE BOOL "Capstone: ARM64 support")
set(CAPSTONE_BUILD_CSTOOL       OFF CACHE BOOL "Capstone: Build cstool")
set(CAPSTONE_BUILD_STATIC       OFF CACHE BOOL "Capstone: Build static library")
set(CAPSTONE_BUILD_TESTS        OFF CACHE BOOL "Capstone: Build tests")
set(CAPSTONE_EVM_SUPPORT        OFF CACHE BOOL "Capstone: EVM support")
set(CAPSTONE_M680X_SUPPORT      OFF CACHE BOOL "Capstone: M680X support")
set(CAPSTONE_M68K_SUPPORT       OFF CACHE BOOL "Capstone: M68K support")
set(CAPSTONE_MIPS_SUPPORT       OFF CACHE BOOL "Capstone: MIPS support")
set(CAPSTONE_PPC_SUPPORT        OFF CACHE BOOL "Capstone: PowerPC support")
set(CAPSTONE_SPARC_SUPPORT      OFF CACHE BOOL "Capstone: Sparc support")
set(CAPSTONE_SYSZ_SUPPORT       OFF CACHE BOOL "Capstone: SystemZ support")
set(CAPSTONE_TMS320C64X_SUPPORT OFF CACHE BOOL "Capstone: TMS320C64X support")
set(CAPSTONE_XCORE_SUPPORT      OFF CACHE BOOL "Capstone: XCore support")

add_subdirectory("capstone/")
add_subdirectory("catch2/")

target_include_directories(capstone-shared INTERFACE "capstone/include/")

add_library(capstone ALIAS capstone-shared)
add_library(catch2 ALIAS Catch2)

add_subdirectory("disassembler/")
