//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/http/header/if_none_match.hpp>

using namespace fitoria;

using http::header::entity_tag;
using http::header::if_none_match;

TEST_SUITE_BEGIN("[fitoria.http.header.if_none_match]");

TEST_CASE("parse")
{
  {
    auto m = if_none_match::parse(" * ");
    CHECK(m->is_any());
    CHECK_EQ(m->etags(), std::vector<entity_tag> {});
  }
  {
    auto m = if_none_match::parse(R"("")");
    CHECK(!m->is_any());
    CHECK_EQ(m->etags(),
             std::vector<entity_tag> {
                 entity_tag(true, ""),
             });
  }
  {
    auto m = if_none_match::parse(R"("abc", W/"cde")");
    CHECK(!m->is_any());
    CHECK_EQ(m->etags(),
             std::vector<entity_tag> {
                 entity_tag(true, "abc"),
                 entity_tag(false, "cde"),
             });
  }
  {
    CHECK(!if_none_match::parse(""));
    CHECK(!if_none_match::parse(R"("abc""cde")"));
    CHECK(!if_none_match::parse(R"("abc"cde")"));
  }
}

TEST_SUITE_END();
