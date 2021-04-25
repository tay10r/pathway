cmake_minimum_required(VERSION 3.9.6)

find_package(FLEX REQUIRED)

find_package(BISON REQUIRED)

set(outdir "${CMAKE_CURRENT_BINARY_DIR}/generated")

file(MAKE_DIRECTORY "${outdir}")

flex_target(ptc_lex "${CMAKE_CURRENT_SOURCE_DIR}/lex.l"
  "${outdir}/lex.cpp"
  DEFINES_FILE "${outdir}/lex.h")

bison_target(ptc_parse "${CMAKE_CURRENT_SOURCE_DIR}/parse.y"
  "${outdir}/parse.cpp"
  DEFINES_FILE "${outdir}/parse.h")

add_executable(ptc
  ${FLEX_ptc_lex_OUTPUTS}
  ${BISON_ptc_parse_OUTPUTS}
  common.h
  common.cpp
  cpp_generator.h
  cpp_generator.cpp
  resolve.h
  resolve.cpp
  check.h
  check.cpp
  main.cpp)

if(NOT MSVC)
  target_compile_options(ptc PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
endif(NOT MSVC)

target_include_directories(ptc PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${outdir}")

set_target_properties(ptc
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
