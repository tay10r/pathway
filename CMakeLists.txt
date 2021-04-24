cmake_minimum_required(VERSION 3.9.6)

project(pathway)

option(PATHWAY_EXAMPLES "Whether or not to build the examples." OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(transpiler)

if(PATHWAY_EXAMPLES)
  add_subdirectory(examples)
endif(PATHWAY_EXAMPLES)
