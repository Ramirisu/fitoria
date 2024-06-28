//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/http/header/entity_tag.hpp>

using namespace fitoria;
using namespace fitoria::http;

TEST_SUITE_BEGIN("[fitoria.http.header.entity_tag]");

TEST_CASE("strong")
{
  CHECK_EQ(http::header::entity_tag::strong("abc"), R"("abc")");
}

TEST_CASE("weak")
{
  CHECK_EQ(http::header::entity_tag::weak("abc"), R"(W/"abc")");
}

TEST_SUITE_END();
