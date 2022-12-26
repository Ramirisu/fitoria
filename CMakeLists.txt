cmake_minimum_required(VERSION 3.15)

project(
  fitoria
  VERSION 0.0.0
  LANGUAGES CXX)

option(FITORIA_BUILD_EXAMPLES "Build examples" OFF)
option(FITORIA_BUILD_TESTS "Build tests" OFF)
option(FITORIA_DISABLE_OPENSSL "Do not use OpenSSL, disable the auto detection"
       OFF)

message(STATUS "[fitoria] FITORIA_BUILD_EXAMPLES = ${FITORIA_BUILD_EXAMPLES}")
message(STATUS "[fitoria] FITORIA_BUILD_TESTS = ${FITORIA_BUILD_TESTS}")
message(STATUS "[fitoria] FITORIA_DISABLE_OPENSSL = ${FITORIA_DISABLE_OPENSSL}")

include(cmake/CPM.cmake)
include(cmake/fitoria_utils.cmake)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

cpmaddpackage("gh:Ramirisu/gul#298c49e")
cpmaddpackage("gh:boostorg/boost#boost-1.81.0")

if(NOT FITORIA_DISABLE_OPENSSL)
  message(STATUS "[fitoria] try to find OpenSSL")
  find_package(OpenSSL)
  message(STATUS "[fitoria] OpenSSL found = ${OPENSSL_FOUND}")
endif()

add_library(fitoria INTERFACE)
target_link_libraries(fitoria INTERFACE gul Boost::asio Boost::beast Boost::url
                                        Boost::json)
if(OPENSSL_FOUND)
  target_compile_definitions(fitoria INTERFACE FITORIA_HAS_OPENSSL)
  target_link_libraries(fitoria INTERFACE OpenSSL::SSL)
endif()
target_include_directories(fitoria
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(FITORIA_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

if(FITORIA_BUILD_TESTS)
  cpmaddpackage("gh:doctest/doctest@2.4.9")

  enable_testing()
  add_subdirectory(test)
endif()
