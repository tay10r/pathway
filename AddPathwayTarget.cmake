function(add_pathway_target)

  if(NOT TARGET ptc)
    message(FATAL_ERROR "Missing 'ptc' executable.")
  endif(NOT TARGET ptc)

  set(options)

  set(single_value_options OUTPUT_FILE DIRECTORY LANGUAGE)

  set(multi_value_options)

  cmake_parse_arguments(ptc_opts
    "${options}"
    "${single_value_options}"
    "${multi_value_options}"
    ${ARGN})

  if(NOT (DEFINED ptc_opts_LANGUAGE))
    message(FATAL_ERROR "Specify what language to generate with 'LANGUAGE'")
  endif(NOT (DEFINED ptc_opts_LANGUAGE))

  if(NOT DEFINED ptc_opts_DIRECTORY)
    message(FATAL_ERROR "Specify where the sources are with 'DIRECTORY'")
  endif(NOT DEFINED ptc_opts_DIRECTORY)

  if(NOT IS_ABSOLUTE "${ptc_opts_DIRECTORY}")
    set(ptc_opts_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${ptc_opts_DIRECTORY}")
  endif(NOT IS_ABSOLUTE "${ptc_opts_DIRECTORY}")

  if(NOT DEFINED ptc_opts_OUTPUT_FILE)
    message(FATAL_ERROR "Specify the output path with 'OUTPUT_FILE'")
  endif(NOT DEFINED ptc_opts_OUTPUT_FILE)

  if(NOT IS_ABSOLUTE "${ptc_opts_OUTPUT_FILE}")
    set(ptc_opts_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${ptc_opts_OUTPUT_FILE}")
  endif(NOT IS_ABSOLUTE "${ptc_opts_OUTPUT_FILE}")

  string(TOLOWER "${ptc_opts_LANGUAGE}" lang)

  if(lang STREQUAL cxx)

    add_custom_command(OUTPUT "${ptc_opts_OUTPUT_FILE}"
      COMMAND $<TARGET_FILE:ptc> "${ptc_opts_DIRECTORY}" --language "${lang}" -o "${ptc_opts_OUTPUT_FILE}")

  else(lang STREQUAL cxx)
    message(FATAL_ERROR "'${ptc_opts_LANGUAGE}' is not a supported language.")
  endif(lang STREQUAL cxx)

endfunction(add_pathway_target)
