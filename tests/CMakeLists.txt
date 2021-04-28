cmake_minimum_required(VERSION 3.9.6)

set(tests
  test_1
  test_2)

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/images")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/diffs")

foreach(test ${tests})

  set(cpp_source "${CMAKE_CURRENT_BINARY_DIR}/${test}.h")

  set(good_image_path "${CMAKE_CURRENT_SOURCE_DIR}/${test}/expected.png")

  set(diff_image_path "${PROJECT_BINARY_DIR}/diffs/${test}.png")

  set(test_image_path "${PROJECT_BINARY_DIR}/images/${test}.png")

  add_custom_target(ptc_${test}_cpp_source ALL
    COMMAND $<TARGET_FILE:ptc> -o ${cpp_source} ${test} --only-if-different
    COMMENT "Generating C++ source for ${test}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

  add_executable(ptc_${test} main.cpp)

  target_compile_definitions(ptc_${test}
    PRIVATE
      "HEADER=\"${cpp_source}\""
      "GOOD_IMAGE_PATH=\"${good_image_path}\""
      "DIFF_IMAGE_PATH=\"${diff_image_path}\""
      "TEST_IMAGE_PATH=\"${test_image_path}\"")

  target_include_directories(ptc_${test} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

  add_dependencies(ptc_${test} ptc_${test}_cpp_source)

  add_test(NAME ptc_${test} COMMAND $<TARGET_FILE:ptc_${test}>)

  set_target_properties(ptc_${test}
    PROPERTIES
      OUTPUT_NAME run_${test}
      RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

endforeach(test ${tests})

enable_testing()
