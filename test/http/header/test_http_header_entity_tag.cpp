//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/http/header/entity_tag.hpp>

using namespace fitoria;

using http::header::entity_tag;

TEST_SUITE_BEGIN("[fitoria.http.header.entity_tag]");

TEST_CASE("strong")
{
  CHECK_EQ(entity_tag::make_strong("abc").to_string(), R"("abc")");
  CHECK_EQ(entity_tag(true, "abc").to_string(), R"("abc")");
  CHECK_EQ(entity_tag(false, "").set_strong(true).set_value("abc").to_string(),
           R"("abc")");
}

TEST_CASE("weak")
{
  CHECK_EQ(entity_tag::make_weak("abc").to_string(), R"(W/"abc")");
  CHECK_EQ(entity_tag(false, "abc").to_string(), R"(W/"abc")");
  CHECK_EQ(entity_tag(true, "").set_strong(false).set_value("abc").to_string(),
           R"(W/"abc")");
}

TEST_CASE("compare")
{
  CHECK_EQ(entity_tag(true, "abc"), entity_tag(true, "abc"));
  CHECK_EQ(entity_tag(false, "abc"), entity_tag(false, "abc"));
  CHECK_NE(entity_tag(true, "abc"), entity_tag(false, "abc"));
  CHECK_NE(entity_tag(false, "abc"), entity_tag(true, "abc"));

  CHECK_NE(entity_tag(true, "abc"), entity_tag(true, "cde"));
  CHECK_NE(entity_tag(false, "abc"), entity_tag(false, "cde"));
  CHECK_NE(entity_tag(true, "abc"), entity_tag(false, "cde"));
  CHECK_NE(entity_tag(false, "abc"), entity_tag(true, "cde"));

  CHECK(entity_tag(true, "abc").weakly_equal_to(entity_tag(true, "abc")));
  CHECK(entity_tag(false, "abc").weakly_equal_to(entity_tag(false, "abc")));
  CHECK(entity_tag(true, "abc").weakly_equal_to(entity_tag(false, "abc")));
  CHECK(entity_tag(false, "abc").weakly_equal_to(entity_tag(true, "abc")));

  CHECK(!entity_tag(true, "abc").weakly_equal_to(entity_tag(true, "cde")));
  CHECK(!entity_tag(false, "abc").weakly_equal_to(entity_tag(false, "cde")));
  CHECK(!entity_tag(true, "abc").weakly_equal_to(entity_tag(false, "cde")));
  CHECK(!entity_tag(false, "abc").weakly_equal_to(entity_tag(true, "cde")));
}

TEST_CASE("parse")
{
  {
    auto etag = http::header::entity_tag::parse(R"("")");
    CHECK(etag->strong());
    CHECK_EQ(etag->value(), "");
  }
  {
    auto etag = http::header::entity_tag::parse(R"("abc")");
    CHECK(etag->strong());
    CHECK_EQ(etag->value(), "abc");
  }
  {
    auto etag = http::header::entity_tag::parse(R"(W/"abc")");
    CHECK(!etag->strong());
    CHECK_EQ(etag->value(), "abc");
  }
  {
    CHECK(!http::header::entity_tag::parse(""));
    CHECK(!http::header::entity_tag::parse(R"(")"));
    CHECK(!http::header::entity_tag::parse(R"(a"")"));
    CHECK(!http::header::entity_tag::parse(R"(""a)"));
    CHECK(!http::header::entity_tag::parse(R"(""")"));
  }
}

TEST_SUITE_END();
