//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/compile_time_path_checker.hpp>

using namespace fitoria::web;

TEST_SUITE_BEGIN("web.compile_time_path_checker");

TEST_CASE("is_valid")
{
  static_assert(compile_time_path_checker::is_valid<"">());
  static_assert(compile_time_path_checker::is_valid<"/">());
  static_assert(compile_time_path_checker::is_valid<"///">());
  static_assert(compile_time_path_checker::is_valid<"/abc/{abc}">());
  static_assert(compile_time_path_checker::is_valid<"/abc/{abcd}/xyz/{xyz}">());
  static_assert(!compile_time_path_checker::is_valid<"{">());
  static_assert(!compile_time_path_checker::is_valid<"}">());
  static_assert(!compile_time_path_checker::is_valid<"/{}">());
  static_assert(!compile_time_path_checker::is_valid<"/{abc">());
  static_assert(!compile_time_path_checker::is_valid<"/abc}">());
  static_assert(!compile_time_path_checker::is_valid<"/{abc">());
  static_assert(!compile_time_path_checker::is_valid<"/{abc}}">());
  static_assert(!compile_time_path_checker::is_valid<"/{abc}xyz">());
  static_assert(!compile_time_path_checker::is_valid<"/{abc}/{abc}">());
  static_assert(!compile_time_path_checker::is_valid<"/{abc}/xyz/{abc}/abc">());
}

TEST_SUITE_END();
