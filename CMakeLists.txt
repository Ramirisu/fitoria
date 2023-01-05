cmake_minimum_required(VERSION 3.15)

project(
  fitoria
  VERSION 0.0.0
  LANGUAGES CXX)

option(FITORIA_BUILD_EXAMPLES "Build examples" OFF)
option(FITORIA_BUILD_TESTS "Build tests" OFF)
option(FITORIA_DISABLE_OPENSSL "Do not use OpenSSL" OFF)
option(FITORIA_DISABLE_CPM "Do not use CPM.cmake to download library" OFF)

message(STATUS "[fitoria] FITORIA_BUILD_EXAMPLES = ${FITORIA_BUILD_EXAMPLES}")
message(STATUS "[fitoria] FITORIA_BUILD_TESTS = ${FITORIA_BUILD_TESTS}")
message(STATUS "[fitoria] FITORIA_DISABLE_OPENSSL = ${FITORIA_DISABLE_OPENSSL}")

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
  find_package(fmt)
  message(STATUS "[fitoria] [dep.lib] fmt found = ${fmt_FOUND}")
  if(fmt_FOUND)
    set(FITORIA_USE_FMT ON)
  else()
    if(NOT FITORIA_DISABLE_CPM)
      message(STATUS "[fitoria] [dep.lib] trying to download fmt by CPM.cmake")
      cpmaddpackage("gh:fmtlib/fmt#9.1.0")
      set(FITORIA_USE_FMT ON)
    else()
      message(FATAL_ERROR "[fitoria] [dep.lib] requires formatting library")
    endif()
  endif()
endif()

# [dep.lib] boost
message(STATUS "[fitoria] [dep.lib] trying to find Boost 1.81.0")
find_package(Boost 1.81.0 COMPONENTS asio beast json url)
message(STATUS "[fitoria] [dep.lib] Boost found = ${BOOST_FOUND}")
if((NOT BOOST_FOUND) AND (NOT FITORIA_DISABLE_CPM))
  message(STATUS "[fitoria] [dep.lib] trying to download Boost by CPM.cmake")
  cpmaddpackage("gh:boostorg/boost#boost-1.81.0")
endif()

# [dep.lib] openssl
if(NOT FITORIA_DISABLE_OPENSSL)
  message(STATUS "[fitoria] [dep.lib] trying to find OpenSSL")
  find_package(OpenSSL)
  message(STATUS "[fitoria] [dep.lib] OpenSSL found = ${OPENSSL_FOUND}")
  if(OPENSSL_FOUND)
    set(FITORIA_USE_OPEN_SSL)
  endif()
endif()

add_library(fitoria INTERFACE)
add_library(fitoria::fitoria ALIAS fitoria)
target_link_libraries(fitoria INTERFACE Boost::asio Boost::beast Boost::json
                                        Boost::url)
target_include_directories(fitoria
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(FITORIA_USE_FMT)
  target_link_libraries(fitoria INTERFACE fmt::fmt)
endif()
if(FITORIA_USE_OPEN_SSL)
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
  find_package(doctest)
  message(STATUS "[fitoria] [dep.lib] doctest found = ${doctest_FOUND}")
  if((NOT doctest_FOUND) AND (NOT FITORIA_DISABLE_CPM))
    message(
      STATUS "[fitoria] [dep.lib] trying to download doctest by CPM.cmake")
    cpmaddpackage("gh:doctest/doctest@2.4.9")
  endif()

  enable_testing()
  add_subdirectory(test)
endif()
