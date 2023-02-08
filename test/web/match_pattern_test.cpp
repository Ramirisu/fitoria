//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/match_pattern.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("web.match_pattern");

TEST_CASE("escape_segment")
{
  CHECK(!match_pattern::escape_segment("{"));
  CHECK(!match_pattern::escape_segment("}"));
  CHECK(!match_pattern::escape_segment("}{"));
  CHECK(!match_pattern::escape_segment("{{}"));
  CHECK(!match_pattern::escape_segment("{}{"));

  CHECK_EQ(match_pattern::escape_segment(""), "");
  CHECK_EQ(match_pattern::escape_segment("{}"), "");
  CHECK_EQ(match_pattern::escape_segment("{abc}"), "abc");
}

TEST_CASE("from_pattern")
{
  CHECK(!match_pattern::from_pattern("/{"));
  CHECK(!match_pattern::from_pattern("/}"));
  CHECK(!match_pattern::from_pattern("{}"));
  CHECK(!match_pattern::from_pattern("/x/{"));
  CHECK(!match_pattern::from_pattern("/x/}"));
  CHECK(!match_pattern::from_pattern("/x/{x}/{"));
  CHECK(!match_pattern::from_pattern("/x/{x}/}"));
  CHECK(!match_pattern::from_pattern("/x/{x}/{{"));
  CHECK(!match_pattern::from_pattern("/x/{x}/}}"));
  CHECK(!match_pattern::from_pattern("/x/{x}/}{"));
}

TEST_CASE("match")
{
  CHECK_EQ(match_pattern::from_pattern("").value().match(""), query_map());
  CHECK_EQ(
      match_pattern::from_pattern("/{1}/2/{3}/4").value().match("/w/x/y/z"),
      query_map {
          { "1", "w" },
          { "3", "y" },
      });
  CHECK(!match_pattern::from_pattern("/1").value().match(""));
}

TEST_SUITE_END();
