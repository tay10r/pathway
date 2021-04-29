cmake_minimum_required(VERSION 3.9.6)

file(GLOB entries "${CMAKE_CURRENT_SOURCE_DIR}/*")

set(tests)

foreach(entry ${entries})

  if(IS_DIRECTORY "${entry}")
    list(APPEND tests "${entry}")
  endif(IS_DIRECTORY "${entry}")

endforeach(entry ${entries})

foreach(test ${tests})

  get_filename_component(name "${test}" NAME_WE)

  add_test(NAME "pathway_error_test:${name}"
    COMMAND $<TARGET_FILE:ptc> --error-test
    WORKING_DIRECTORY "${test}")

endforeach(test ${tests})

enable_testing()
