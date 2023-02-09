//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/pattern_matcher.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("web.pattern_matcher");

TEST_CASE("escape_segment")
{
  CHECK(!pattern_matcher::escape_segment("{"));
  CHECK(!pattern_matcher::escape_segment("}"));
  CHECK(!pattern_matcher::escape_segment("}{"));
  CHECK(!pattern_matcher::escape_segment("{{}"));
  CHECK(!pattern_matcher::escape_segment("{}{"));

  CHECK_EQ(pattern_matcher::escape_segment(""), "");
  CHECK_EQ(pattern_matcher::escape_segment("{}"), "");
  CHECK_EQ(pattern_matcher::escape_segment("{abc}"), "abc");
}

TEST_CASE("from_pattern")
{
  CHECK(!pattern_matcher::from_pattern("/{"));
  CHECK(!pattern_matcher::from_pattern("/}"));
  CHECK(!pattern_matcher::from_pattern("{}"));
  CHECK(!pattern_matcher::from_pattern("/x/{"));
  CHECK(!pattern_matcher::from_pattern("/x/}"));
  CHECK(!pattern_matcher::from_pattern("/x/{x}/{"));
  CHECK(!pattern_matcher::from_pattern("/x/{x}/}"));
  CHECK(!pattern_matcher::from_pattern("/x/{x}/{{"));
  CHECK(!pattern_matcher::from_pattern("/x/{x}/}}"));
  CHECK(!pattern_matcher::from_pattern("/x/{x}/}{"));
}

TEST_CASE("match")
{
  CHECK_EQ(pattern_matcher::from_pattern("").value().match(""), query_map());
  CHECK_EQ(
      pattern_matcher::from_pattern("/{1}/2/{3}/4").value().match("/w/x/y/z"),
      query_map {
          { "1", "w" },
          { "3", "y" },
      });
  CHECK(!pattern_matcher::from_pattern("/1").value().match(""));
}

TEST_SUITE_END();
