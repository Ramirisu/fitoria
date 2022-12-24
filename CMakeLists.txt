cmake_minimum_required(VERSION 3.15)

project(
  fitoria
  VERSION 0.0.0
  LANGUAGES CXX)

option(FITORIA_BUILD_EXAMPLES "Build examples" OFF)
option(FITORIA_BUILD_TESTS "Build tests" OFF)

include(cmake/CPM.cmake)
include(cmake/fitoria_utils.cmake)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

cpmaddpackage("gh:Ramirisu/gul#298c49e")
cpmaddpackage("gh:boostorg/boost#boost-1.81.0")

find_package(OpenSSL REQUIRED)

add_library(fitoria INTERFACE)
target_link_libraries(fitoria INTERFACE gul OpenSSL::SSL Boost::asio
                                        Boost::beast Boost::url Boost::json)
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
