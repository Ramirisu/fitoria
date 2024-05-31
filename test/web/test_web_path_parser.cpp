//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/web/path_parser.hpp>

using namespace fitoria::web;

#define TEST_PARSE_CT_AND_RT(wildcard, path, success)                          \
  static_assert(path_parser<wildcard>().parse<path>() == (success));           \
  CHECK(path_parser<wildcard>().parse(path) == (success));

TEST_SUITE_BEGIN("[fitoria.web.path_parser]");

TEST_CASE("without wildcard support")
{
  TEST_PARSE_CT_AND_RT(false, "", true);
  TEST_PARSE_CT_AND_RT(false, "/", true);
  TEST_PARSE_CT_AND_RT(false, "///", true);
  TEST_PARSE_CT_AND_RT(false, "/abc/{abc}", true);
  TEST_PARSE_CT_AND_RT(false, "/abc/{abcd}/xyz/{xyz}", true);
  TEST_PARSE_CT_AND_RT(false, "/%00/%FF/%9A", true);
  TEST_PARSE_CT_AND_RT(false, "/abcdefghijklmnopqrstuvwxyz", true);
  TEST_PARSE_CT_AND_RT(false, "/ABCDEFGHIJKLMNOPQRSTUVWXYZ", true);
  TEST_PARSE_CT_AND_RT(false, "/0123456789-._~", true);
  TEST_PARSE_CT_AND_RT(false, "/!$&'()*+,;=", true);
  TEST_PARSE_CT_AND_RT(false, "{", false);
  TEST_PARSE_CT_AND_RT(false, "}", false);
  TEST_PARSE_CT_AND_RT(false, "{}", false);
  TEST_PARSE_CT_AND_RT(false, "/{}", false);
  TEST_PARSE_CT_AND_RT(false, "/{abc", false);
  TEST_PARSE_CT_AND_RT(false, "/abc}", false);
  TEST_PARSE_CT_AND_RT(false, "/{abc}}", false);
  TEST_PARSE_CT_AND_RT(false, "/{abc}xyz", false);
  TEST_PARSE_CT_AND_RT(false, "/{abc}/{abc}", false);
  TEST_PARSE_CT_AND_RT(false, "/{abc}/xyz/{abc}/abc", false);
  TEST_PARSE_CT_AND_RT(false, "/{abc/xyz}", false);
  TEST_PARSE_CT_AND_RT(false, "/{abc}.{xyz}", false);
  TEST_PARSE_CT_AND_RT(false, "/{abc}xyz", false);
  TEST_PARSE_CT_AND_RT(false, "/xyz{abc}", false);
  TEST_PARSE_CT_AND_RT(false, "/%GC", false);
  TEST_PARSE_CT_AND_RT(false, "/%AG", false);

  TEST_PARSE_CT_AND_RT(false, "/#abc", false);
  TEST_PARSE_CT_AND_RT(false, "/abc/#xyz", false);
  TEST_PARSE_CT_AND_RT(false, "/abc#xyz", false);
  TEST_PARSE_CT_AND_RT(false, "/#", false);
  TEST_PARSE_CT_AND_RT(false, "/{abc}/#abc", false);
  TEST_PARSE_CT_AND_RT(false, "/abc#/x", false);
  TEST_PARSE_CT_AND_RT(false, "/#/abc", false);
  TEST_PARSE_CT_AND_RT(false, "/{#}", false);
  TEST_PARSE_CT_AND_RT(false, "/#{abc}", false);
}

TEST_CASE("with wildcard support")
{
  TEST_PARSE_CT_AND_RT(true, "", true);
  TEST_PARSE_CT_AND_RT(true, "/", true);
  TEST_PARSE_CT_AND_RT(true, "///", true);
  TEST_PARSE_CT_AND_RT(true, "/abc/{abc}", true);
  TEST_PARSE_CT_AND_RT(true, "/abc/{abcd}/xyz/{xyz}", true);
  TEST_PARSE_CT_AND_RT(true, "/%00/%FF/%9A", true);
  TEST_PARSE_CT_AND_RT(true, "/abcdefghijklmnopqrstuvwxyz", true);
  TEST_PARSE_CT_AND_RT(true, "/ABCDEFGHIJKLMNOPQRSTUVWXYZ", true);
  TEST_PARSE_CT_AND_RT(true, "/0123456789-._~", true);
  TEST_PARSE_CT_AND_RT(true, "/!$&'()*+,;=", true);
  TEST_PARSE_CT_AND_RT(true, "{", false);
  TEST_PARSE_CT_AND_RT(true, "}", false);
  TEST_PARSE_CT_AND_RT(true, "{}", false);
  TEST_PARSE_CT_AND_RT(true, "/{}", false);
  TEST_PARSE_CT_AND_RT(true, "/{abc", false);
  TEST_PARSE_CT_AND_RT(true, "/abc}", false);
  TEST_PARSE_CT_AND_RT(true, "/{abc}}", false);
  TEST_PARSE_CT_AND_RT(true, "/{abc}xyz", false);
  TEST_PARSE_CT_AND_RT(true, "/{abc}/{abc}", false);
  TEST_PARSE_CT_AND_RT(true, "/{abc}/xyz/{abc}/abc", false);
  TEST_PARSE_CT_AND_RT(true, "/{abc/xyz}", false);
  TEST_PARSE_CT_AND_RT(true, "/{abc}.{xyz}", false);
  TEST_PARSE_CT_AND_RT(true, "/{abc}xyz", false);
  TEST_PARSE_CT_AND_RT(true, "/xyz{abc}", false);
  TEST_PARSE_CT_AND_RT(true, "/%GC", false);
  TEST_PARSE_CT_AND_RT(true, "/%AG", false);

  TEST_PARSE_CT_AND_RT(true, "/#abc", true);
  TEST_PARSE_CT_AND_RT(true, "/abc/#xyz", true);
  TEST_PARSE_CT_AND_RT(true, "/abc#xyz", true);
  TEST_PARSE_CT_AND_RT(true, "/#", false);
  TEST_PARSE_CT_AND_RT(true, "/{abc}/#abc", false);
  TEST_PARSE_CT_AND_RT(true, "/abc#/x", false);
  TEST_PARSE_CT_AND_RT(true, "/#/abc", false);
  TEST_PARSE_CT_AND_RT(true, "/{#}", false);
  TEST_PARSE_CT_AND_RT(true, "/#{abc}", false);
}

TEST_SUITE_END();
