function(fitoria_target_compile_option target_name)
  target_link_libraries(${target_name} PRIVATE fitoria)
  if((CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
     OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC"))
    target_compile_options(${target_name} PRIVATE /bigobj /W4 /WX)
  else()
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Werror
                                                  -pedantic -pedantic-errors)
    if(FITORIA_ENABLE_CODECOV)
      target_compile_options(${target_name} PRIVATE --coverage)
      target_link_libraries(${target_name} PRIVATE gcov)
    endif()
  endif()
endfunction()

# fitoria_add_example(NAME name SRC [source_files...])
function(fitoria_add_example)
  cmake_parse_arguments(PARSED_ARGS "" "NAME" "SRCS" ${ARGN})
  if(NOT PARSED_ARGS_NAME)
    message(FATAL_ERROR "NAME must be provied")
  endif()
  set(target_name "${PARSED_ARGS_NAME}_example")
  add_executable(${target_name} ${PARSED_ARGS_SRCS})
  fitoria_target_compile_option(${target_name})
endfunction()
