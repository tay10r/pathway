cmake_minimum_required(VERSION 3.9.6)

include(../AddPathwayTarget.cmake)

function(add_pathway_example name)

  add_pathway_target(DIRECTORY "${name}" LANGUAGE cxx
    OUTPUT_FILE ${name}.h)

  set(target pathway_example_${name}_main)

  add_executable(${target}
    ${name}/main.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/${name}.h")

  set_target_properties(${target}
    PROPERTIES
      OUTPUT_NAME ${name}
      RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

  target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

  if(NOT MSVC)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
  endif(NOT MSVC)

endfunction(add_pathway_example name)

add_pathway_example(rtweekend)
