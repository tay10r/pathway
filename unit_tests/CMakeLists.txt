cmake_minimum_required(VERSION 3.14.7)

include(FetchContent)

FetchContent_Declare(googletest
  URL "https://github.com/google/googletest/archive/master.zip")

FetchContent_MakeAvailable(googletest)

add_executable(ptc_unit_tests
  runtime.cpp
  diagnostics.cpp
  cpp_expr_generation.cpp
  string_to_expr.h
  string_to_expr.cpp
  lexer.cpp
  type_inference.cpp)

if(NOT MSVC)
  target_compile_options(ptc_unit_tests PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
endif(NOT MSVC)

target_link_libraries(ptc_unit_tests PRIVATE gtest gtest_main ptclib pathway_runtime)

target_include_directories(ptc_unit_tests
  PRIVATE
    "${PROJECT_SOURCE_DIR}/transpiler")

set_target_properties(ptc_unit_tests
  PROPERTIES
    OUTPUT_NAME "run_unit_tests"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

add_test(NAME ptc_unit_tests COMMAND $<TARGET_FILE:ptc_unit_tests>)

enable_testing()
