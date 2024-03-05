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

TEST_CASE("is_valid_scope")
{
  using ctpc = compile_time_path_checker;

  static_assert(ctpc::is_valid_scope<"">());
  static_assert(ctpc::is_valid_scope<"/">());
  static_assert(ctpc::is_valid_scope<"///">());
  static_assert(ctpc::is_valid_scope<"/abc/{abc}">());
  static_assert(ctpc::is_valid_scope<"/abc/{abcd}/xyz/{xyz}">());
  static_assert(ctpc::is_valid_scope<"/%00/%FF/%9A">());
  static_assert(ctpc::is_valid_scope<"/abcdefghijklmnopqrstuvwxyz">());
  static_assert(ctpc::is_valid_scope<"/ABCDEFGHIJKLMNOPQRSTUVWXYZ">());
  static_assert(ctpc::is_valid_scope<"/0123456789-._~">());
  static_assert(ctpc::is_valid_scope<"/!$&'()*+,;=">());
  static_assert(!ctpc::is_valid_scope<"{">());
  static_assert(!ctpc::is_valid_scope<"}">());
  static_assert(!ctpc::is_valid_scope<"/{}">());
  static_assert(!ctpc::is_valid_scope<"/{abc">());
  static_assert(!ctpc::is_valid_scope<"/abc}">());
  static_assert(!ctpc::is_valid_scope<"/{abc}}">());
  static_assert(!ctpc::is_valid_scope<"/{abc}xyz">());
  static_assert(!ctpc::is_valid_scope<"/{abc}/{abc}">());
  static_assert(!ctpc::is_valid_scope<"/{abc}/xyz/{abc}/abc">());
  static_assert(!ctpc::is_valid_scope<"/{abc/xyz}">());
  static_assert(!ctpc::is_valid_scope<"/{abc}.{xyz}">());
  static_assert(!ctpc::is_valid_scope<"/{abc}.xyz">());
  static_assert(!ctpc::is_valid_scope<"/xyz.{abc}">());
  static_assert(!ctpc::is_valid_scope<"/%GC">());
}

TEST_SUITE_END();
