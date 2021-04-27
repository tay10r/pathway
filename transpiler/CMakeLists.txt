cmake_minimum_required(VERSION 3.9.6)

find_package(BISON REQUIRED)

set(outdir "${CMAKE_CURRENT_BINARY_DIR}/generated")

file(MAKE_DIRECTORY "${outdir}")

bison_target(ptc_parse "${CMAKE_CURRENT_SOURCE_DIR}/parse.y"
  "${outdir}/parse.cpp"
  DEFINES_FILE "${outdir}/parse.h")

add_library(ptclib
  ${BISON_ptc_parse_OUTPUTS}
  abort.h
  abort.cpp
  check.h
  check.cpp
  common.h
  common.cpp
  cpp_generator.h
  cpp_generator.cpp
  expr.h
  expr.cpp
  lexer.h
  lexer.cpp
  location.h
  location.cpp
  parse.h
  parse.cpp
  resolve.h
  resolve.cpp
  type.h
  type.cpp)

target_compile_features(ptclib PUBLIC cxx_std_17)

target_include_directories(ptclib
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}")

add_executable(ptc
  main.cpp)

target_include_directories(ptc
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}")

if(NOT MSVC)

  target_compile_options(ptclib PRIVATE -Wall -Wextra -Werror -Wfatal-errors)

  target_compile_options(ptc PRIVATE -Wall -Wextra -Werror -Wfatal-errors)

endif(NOT MSVC)

set_target_properties(ptc
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

target_link_libraries(ptc PRIVATE ptclib)
