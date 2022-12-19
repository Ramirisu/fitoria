cmake_minimum_required(VERSION 3.15)

project(
  fitoria
  VERSION 0.0.0
  LANGUAGES CXX)

option(FITORIA_BUILD_TESTS "Build tests" OFF)

include(cmake/CPM.cmake)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

cpmaddpackage("gh:Ramirisu/gul#551c6e4")

add_library(fitoria INTERFACE)
target_link_libraries(fitoria INTERFACE gul)
target_include_directories(fitoria
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(FITORIA_BUILD_TESTS)
  cpmaddpackage("gh:doctest/doctest@2.4.9")

  enable_testing()
  add_subdirectory(test)
endif()
