cmake_minimum_required(VERSION 3.17)

project(
  fitoria
  VERSION 0.0.0
  LANGUAGES CXX)

option(FITORIA_BUILD_EXAMPLES "Build examples" OFF)
option(FITORIA_BUILD_TESTS "Build tests" OFF)
option(FITORIA_DISABLE_OPENSSL "Do not use OpenSSL" OFF)
option(FITORIA_DISABLE_ZLIB "Do not use zlib" OFF)
option(FITORIA_DISABLE_BROTLI "Do not use brotli" OFF)
option(FITORIA_ENABLE_CODECOV "Enable codecov build" OFF)

message(STATUS "[fitoria] FITORIA_BUILD_EXAMPLES = ${FITORIA_BUILD_EXAMPLES}")
message(STATUS "[fitoria] FITORIA_BUILD_TESTS = ${FITORIA_BUILD_TESTS}")
message(STATUS "[fitoria] FITORIA_DISABLE_OPENSSL = ${FITORIA_DISABLE_OPENSSL}")
message(STATUS "[fitoria] FITORIA_DISABLE_ZLIB = ${FITORIA_DISABLE_ZLIB}")
message(STATUS "[fitoria] FITORIA_DISABLE_BROTLI = ${FITORIA_DISABLE_BROTLI}")
message(STATUS "[fitoria] FITORIA_ENABLE_CODECOV = ${FITORIA_ENABLE_CODECOV}")

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

include(cmake/fitoria_utils.cmake)

# [dep.lib] format lib
message(STATUS "[fitoria] [dep.lib.format] try to find fmt")
find_package(fmt)
message(STATUS "[fitoria] [dep.lib.format] fmt found = ${fmt_FOUND}")

if(fmt_FOUND)
  set(FITORIA_USE_FMT ON)
else()
  message(FATAL_ERROR "[fitoria] [dep.lib.format] fmt is required")
endif()

# [dep.lib] zlib
if(NOT FITORIA_DISABLE_ZLIB)
  message(STATUS "[fitoria] [dep.lib.zlib] trying to find zlib")
  find_package(ZLIB)
  message(STATUS "[fitoria] [dep.lib.zlib] zlib found = ${ZLIB_FOUND}")

  if(ZLIB_FOUND)
    set(FITORIA_USE_ZLIB ON)
  endif()
endif()

# [dep.lib] brotli
if(NOT FITORIA_DISABLE_BROTLI)
  message(STATUS "[fitoria] [dep.lib.brotli] trying to find brotli")
  find_package(unofficial-brotli)
  message(
    STATUS
      "[fitoria] [dep.lib.brotli] brotli found = ${unofficial-brotli_FOUND}")

  if(unofficial-brotli_FOUND)
    set(FITORIA_USE_BROTLI ON)
  endif()
endif()

# [dep.lib] openssl
if(NOT FITORIA_DISABLE_OPENSSL)
  message(STATUS "[fitoria] [dep.lib.openssl] trying to find OpenSSL")
  find_package(OpenSSL)
  message(STATUS "[fitoria] [dep.lib.openssl] OpenSSL found = ${OPENSSL_FOUND}")

  if(OPENSSL_FOUND)
    set(FITORIA_USE_OPENSSL ON)
  endif()
endif()

# [dep.lib] boost
message(STATUS "[fitoria] [dep.lib.boost] trying to find Boost 1.84.0")
find_package(Boost 1.84.0 REQUIRED regex json url)
message(STATUS "[fitoria] [dep.lib.boost] Boost found = ${BOOST_FOUND}")
if(NOT BOOST_FOUND)
  message(FATAL_ERROR "[fitoria] [dep.lib.boost] boost is required")
endif()

add_library(fitoria INTERFACE)
add_library(fitoria::fitoria ALIAS fitoria)
target_link_libraries(fitoria INTERFACE Boost::regex Boost::json Boost::url)
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

if(FITORIA_USE_ZLIB)
  target_compile_definitions(fitoria INTERFACE FITORIA_HAS_ZLIB)
  target_link_libraries(fitoria INTERFACE ZLIB::ZLIB)
endif()

if(FITORIA_USE_BROTLI)
  target_compile_definitions(fitoria INTERFACE FITORIA_HAS_BROTLI)
  target_link_libraries(fitoria INTERFACE unofficial::brotli::brotlidec
                                          unofficial::brotli::brotlienc)
endif()

if(WIN32)
  target_compile_definitions(fitoria INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if((CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
   OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC"))
  target_compile_definitions(fitoria INTERFACE _CRT_SECURE_NO_WARNINGS)
endif()

if(FITORIA_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

if(FITORIA_BUILD_TESTS)
  message(STATUS "[fitoria] [dep.lib.doctest] trying to find doctest")
  find_package(doctest REQUIRED)
  message(STATUS "[fitoria] [dep.lib.doctest] doctest found = ${doctest_FOUND}")

  enable_testing()
  add_subdirectory(test)
endif()
