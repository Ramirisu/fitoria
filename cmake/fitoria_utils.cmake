function(fitoria_target_compile_option target_name)
  target_link_libraries(${target_name} PRIVATE fitoria)

  if(WIN32)
    target_compile_definitions(${target_name} PRIVATE _WIN32_WINNT=0x0A00)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
     OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC"))
    target_compile_options(${target_name} PRIVATE /utf-8 /EHsc /bigobj /W4 /WX
                                                  $<$<CONFIG:Debug>:/JMC>)
  else()
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Werror
                                                  -pedantic -pedantic-errors)

    if(FITORIA_ENABLE_CODECOV)
      target_compile_options(${target_name} PRIVATE --coverage)
      target_link_libraries(${target_name} PRIVATE gcov)
    endif()
  endif()
endfunction()

function(fitoria_try_target_link_liburing target_name)
  if((CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
     OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC"))

  else()
    find_package(PkgConfig)
    pkg_check_modules(liburing IMPORTED_TARGET GLOBAL liburing>=2.0)
    message(STATUS "[fitoria] [dep.lib] liburing found = ${liburing_FOUND}")
    if(liburing_FOUND)
      target_link_libraries(${target_name} PRIVATE PkgConfig::liburing)
    endif()
  endif()
endfunction()
