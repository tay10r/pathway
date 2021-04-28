cmake_minimum_required(VERSION 3.9.6)

project(pathway)

option(PATHWAY_EXAMPLES "Whether or not to build the examples." OFF)
option(PATHWAY_TESTS    "Whether or not to build the tests." OFF)

add_subdirectory(runtime)
add_subdirectory(transpiler)

if(PATHWAY_EXAMPLES)
  add_subdirectory(examples)
endif(PATHWAY_EXAMPLES)

if(PATHWAY_TESTS)
  #add_subdirectory(tests)
  add_subdirectory(unit_tests)
  enable_testing()
endif(PATHWAY_TESTS)
