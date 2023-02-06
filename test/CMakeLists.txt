# fitoria_add_test(NAME name SRC [source_files...])
function(fitoria_add_test)
  cmake_parse_arguments(PARSED_ARGS "" "NAME" "SRCS" ${ARGN})

  if(NOT PARSED_ARGS_NAME)
    message(FATAL_ERROR "NAME must be provied")
  endif()

  set(target_name "${PARSED_ARGS_NAME}")
  add_executable(${target_name} ${PARSED_ARGS_SRCS})
  fitoria_target_compile_option(${target_name})
  target_link_libraries(${target_name} PRIVATE doctest::doctest)
  target_include_directories(${target_name}
    PRIVATE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/include)
  add_test(${target_name} ${target_name})
endfunction()

add_subdirectory(core)
add_subdirectory(log)
add_subdirectory(encoding)
add_subdirectory(web)
add_subdirectory(client)
add_subdirectory(win32)
