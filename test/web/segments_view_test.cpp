//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/segments_view.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("web.segments_view");

TEST_CASE("escape_segment")
{
  using exp_t = expected<std::string_view, error_code>;

  CHECK_EQ(segments_view::escape_segment("{"),
           exp_t(unexpect, make_error_code(error::route_parse_error)));
  CHECK_EQ(segments_view::escape_segment("}"),
           exp_t(unexpect, make_error_code(error::route_parse_error)));
  CHECK_EQ(segments_view::escape_segment("}{"),
           exp_t(unexpect, make_error_code(error::route_parse_error)));
  CHECK_EQ(segments_view::escape_segment("{{}"),
           exp_t(unexpect, make_error_code(error::route_parse_error)));
  CHECK_EQ(segments_view::escape_segment("{}{"),
           exp_t(unexpect, make_error_code(error::route_parse_error)));

  CHECK_EQ(segments_view::escape_segment(""), exp_t(""));
  CHECK_EQ(segments_view::escape_segment("{}"), exp_t(""));
  CHECK_EQ(segments_view::escape_segment("{abc}"), exp_t("abc"));
}

TEST_CASE("unescape_segment")
{
  CHECK_EQ(segments_view::unescape_segment(""), "{}");
  CHECK_EQ(segments_view::unescape_segment("abc"), "{abc}");
}

TEST_CASE("parse_param_map")
{
  using exp_t = expected<query_map, error_code>;

  CHECK_EQ(segments_view::parse_param_map("", ""), exp_t());
  CHECK_EQ(segments_view::parse_param_map("/{1}/2/{3}/4", "/w/x/y/z"),
           exp_t(query_map {
               { "1", "w" },
               { "3", "y" },
           }));

  CHECK_EQ(segments_view::parse_param_map("/1", ""),
           exp_t(unexpect, make_error_code(error::route_parse_error)));
}

TEST_SUITE_END();
