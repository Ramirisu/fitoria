//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/web/path_parser.hpp>

using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.path_parser]");

TEST_CASE("without wildcard support")
{
  static_assert(path_parser<false>().parse<"">());
  static_assert(path_parser<false>().parse<"/">());
  static_assert(path_parser<false>().parse<"///">());
  static_assert(path_parser<false>().parse<"/abc/{abc}">());
  static_assert(path_parser<false>().parse<"/abc/{abcd}/xyz/{xyz}">());
  static_assert(path_parser<false>().parse<"/%00/%FF/%9A">());
  static_assert(path_parser<false>().parse<"/abcdefghijklmnopqrstuvwxyz">());
  static_assert(path_parser<false>().parse<"/ABCDEFGHIJKLMNOPQRSTUVWXYZ">());
  static_assert(path_parser<false>().parse<"/0123456789-._~">());
  static_assert(path_parser<false>().parse<"/!$&'()*+,;=">());
  static_assert(!path_parser<false>().parse<"{">());
  static_assert(!path_parser<false>().parse<"}">());
  static_assert(!path_parser<false>().parse<"{}">());
  static_assert(!path_parser<false>().parse<"/{}">());
  static_assert(!path_parser<false>().parse<"/{abc">());
  static_assert(!path_parser<false>().parse<"/abc}">());
  static_assert(!path_parser<false>().parse<"/{abc}}">());
  static_assert(!path_parser<false>().parse<"/{abc}xyz">());
  static_assert(!path_parser<false>().parse<"/{abc}/{abc}">());
  static_assert(!path_parser<false>().parse<"/{abc}/xyz/{abc}/abc">());
  static_assert(!path_parser<false>().parse<"/{abc/xyz}">());
  static_assert(!path_parser<false>().parse<"/{abc}.{xyz}">());
  static_assert(!path_parser<false>().parse<"/{abc}xyz">());
  static_assert(!path_parser<false>().parse<"/xyz{abc}">());
  static_assert(!path_parser<false>().parse<"/%GC">());

  static_assert(!path_parser<false>().parse<"/#abc">());
  static_assert(!path_parser<false>().parse<"/abc/#xyz">());
  static_assert(!path_parser<false>().parse<"/abc#xyz">());
  static_assert(!path_parser<false>().parse<"/#">());
  static_assert(!path_parser<false>().parse<"/abc#/x">());
  static_assert(!path_parser<false>().parse<"/#/abc">());
  static_assert(!path_parser<false>().parse<"/{#}">());
  static_assert(!path_parser<false>().parse<"/#{abc}">());
}

TEST_CASE("with wildcard support")
{
  static_assert(path_parser<true>().parse<"">());
  static_assert(path_parser<true>().parse<"/">());
  static_assert(path_parser<true>().parse<"///">());
  static_assert(path_parser<true>().parse<"/abc/{abc}">());
  static_assert(path_parser<true>().parse<"/abc/{abcd}/xyz/{xyz}">());
  static_assert(path_parser<true>().parse<"/%00/%FF/%9A">());
  static_assert(path_parser<true>().parse<"/abcdefghijklmnopqrstuvwxyz">());
  static_assert(path_parser<true>().parse<"/ABCDEFGHIJKLMNOPQRSTUVWXYZ">());
  static_assert(path_parser<true>().parse<"/0123456789-._~">());
  static_assert(path_parser<true>().parse<"/!$&'()*+,;=">());
  static_assert(!path_parser<true>().parse<"{">());
  static_assert(!path_parser<true>().parse<"}">());
  static_assert(!path_parser<true>().parse<"{}">());
  static_assert(!path_parser<true>().parse<"/{}">());
  static_assert(!path_parser<true>().parse<"/{abc">());
  static_assert(!path_parser<true>().parse<"/abc}">());
  static_assert(!path_parser<true>().parse<"/{abc}}">());
  static_assert(!path_parser<true>().parse<"/{abc}xyz">());
  static_assert(!path_parser<true>().parse<"/{abc}/{abc}">());
  static_assert(!path_parser<true>().parse<"/{abc}/xyz/{abc}/abc">());
  static_assert(!path_parser<true>().parse<"/{abc/xyz}">());
  static_assert(!path_parser<true>().parse<"/{abc}.{xyz}">());
  static_assert(!path_parser<true>().parse<"/{abc}xyz">());
  static_assert(!path_parser<true>().parse<"/xyz{abc}">());
  static_assert(!path_parser<true>().parse<"/%GC">());

  static_assert(path_parser<true>().parse<"/#abc">());
  static_assert(path_parser<true>().parse<"/abc/#xyz">());
  static_assert(path_parser<true>().parse<"/abc#xyz">());
  static_assert(!path_parser<true>().parse<"/#">());
  static_assert(!path_parser<true>().parse<"/abc#/x">());
  static_assert(!path_parser<true>().parse<"/#/abc">());
  static_assert(!path_parser<true>().parse<"/{#}">());
  static_assert(!path_parser<true>().parse<"/#{abc}">());
}

TEST_SUITE_END();
