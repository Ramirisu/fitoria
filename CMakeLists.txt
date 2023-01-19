cmake_minimum_required(VERSION 3.17)

project(
  fitoria
  VERSION 0.0.0
  LANGUAGES CXX)

option(FITORIA_BUILD_EXAMPLES "Build examples" OFF)
option(FITORIA_BUILD_TESTS "Build tests" OFF)
option(FITORIA_DISABLE_OPENSSL "Do not use OpenSSL" OFF)
option(FITORIA_ENABLE_CODECOV "Enable codecov build" OFF)

message(STATUS "[fitoria] FITORIA_BUILD_EXAMPLES = ${FITORIA_BUILD_EXAMPLES}")
message(STATUS "[fitoria] FITORIA_BUILD_TESTS = ${FITORIA_BUILD_TESTS}")
message(STATUS "[fitoria] FITORIA_DISABLE_OPENSSL = ${FITORIA_DISABLE_OPENSSL}")
message(STATUS "[fitoria] FITORIA_ENABLE_CODECOV = ${FITORIA_ENABLE_CODECOV}")

include(cmake/CPM.cmake)
include(cmake/fitoria_utils.cmake)
include(cmake/fitoria_check_compiler_feature.cmake)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# [dep.lib] std.format
detect_std_format()
message(
  STATUS
    "[fitoria] [dep.lib] std.format = ${FITORIA_CXX_COMPILER_HAS_STD_FORMAT}")
if(NOT FITORIA_CXX_COMPILER_HAS_STD_FORMAT)
  message(STATUS "[fitoria] [dep.lib] try to find fmt")
  find_package(fmt REQUIRED)
  message(STATUS "[fitoria] [dep.lib] fmt found = ${fmt_FOUND}")
  if(fmt_FOUND)
    set(FITORIA_USE_FMT ON)
  endif()
endif()

# [dep.lib] boost
message(STATUS "[fitoria] [dep.lib] trying to find Boost 1.81.0")
find_package(Boost 1.81.0 REQUIRED json url)
message(STATUS "[fitoria] [dep.lib] Boost found = ${BOOST_FOUND}")

# [dep.lib] openssl
if(NOT FITORIA_DISABLE_OPENSSL)
  message(STATUS "[fitoria] [dep.lib] trying to find OpenSSL")
  find_package(OpenSSL)
  message(STATUS "[fitoria] [dep.lib] OpenSSL found = ${OPENSSL_FOUND}")
  if(OPENSSL_FOUND)
    set(FITORIA_USE_OPENSSL ON)
  endif()
endif()

add_library(fitoria INTERFACE)
add_library(fitoria::fitoria ALIAS fitoria)
target_link_libraries(fitoria INTERFACE Boost::json Boost::url)
target_link_directories(fitoria INTERFACE ${Boost_LIBRARY_DIRS})
target_include_directories(fitoria INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                             ${Boost_INCLUDE_DIRS})
if(FITORIA_USE_FMT)
  target_link_libraries(fitoria INTERFACE fmt::fmt)
endif()
if(FITORIA_USE_OPENSSL)
  target_compile_definitions(fitoria INTERFACE FITORIA_HAS_OPENSSL)
  target_link_libraries(fitoria INTERFACE OpenSSL::SSL)
endif()
if(WIN32)
  target_compile_definitions(fitoria INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_definitions(fitoria INTERFACE _CRT_SECURE_NO_WARNINGS)
endif()

if(FITORIA_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

if(FITORIA_BUILD_TESTS)
  message(STATUS "[fitoria] [dep.lib] trying to find doctest")
  find_package(doctest REQUIRED)
  message(STATUS "[fitoria] [dep.lib] doctest found = ${doctest_FOUND}")

  enable_testing()
  add_subdirectory(test)
endif()
