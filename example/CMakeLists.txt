# fitoria_add_example(NAME name SRC [source_files...])
function(fitoria_add_example)
  cmake_parse_arguments(PARSED_ARGS "" "NAME" "SRCS" ${ARGN})
  if(NOT PARSED_ARGS_NAME)
    message(FATAL_ERROR "NAME must be provied")
  endif()
  set(target_name "${PARSED_ARGS_NAME}")
  add_executable(${target_name} ${PARSED_ARGS_SRCS})
  fitoria_target_compile_option(${target_name})
  target_include_directories(${target_name}
                             PRIVATE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/include)
endfunction()

add_subdirectory(http_server)
