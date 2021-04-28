cmake_minimum_required(VERSION 3.9.6)

add_library(pathway_runtime INTERFACE)

target_include_directories(pathway_runtime INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
