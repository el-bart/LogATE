cmake_minimum_required(VERSION 3.0)
project(LogATE-all)

set(LOGATE_ENABLE_TESTS     OFF CACHE BOOL   "build tests (useful for development. to be turned off when preparing installation builds)")
set(LOGATE_SANITIZE         ""  CACHE STRING "enabled one of the sanitizers: address, thread, undefined")
set(LOGATE_ENABLE_PIC       OFF CACHE BOOL   "build code with Place-Independent-Code flag")
set(LOGATE_ENABLE_COVERAGE  OFF CACHE BOOL   "build code with code-coverage flags")

include(cmake/flags/${COMPILER_FLAGS}.cmake)

if(LOGATE_ENABLE_TESTS)
  message(STATUS "LogATE: enabling tests")
  add_subdirectory(/usr/src/googletest ${CMAKE_CURRENT_BINARY_DIR}/gtest)
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/gtest)
  enable_testing()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(LogATE)
