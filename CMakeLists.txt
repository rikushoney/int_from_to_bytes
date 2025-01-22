cmake_minimum_required(VERSION 3.20)

project(int_from_to_bytes LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_library(int_from_to_bytes INTERFACE)
target_sources(int_from_to_bytes INTERFACE int_from_to_bytes.h)

if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
  enable_testing()
  add_executable(examples examples.cpp)
  add_test(NAME test_examples COMMAND $<TARGET_FILE:examples>)
endif()
