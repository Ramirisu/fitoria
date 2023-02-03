//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/query_map.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("web.query_map");

TEST_CASE("basic")
{
  auto m = query_map();
  const auto& c = m;

  CHECK(m.empty());
  CHECK_EQ(m.size(), 0);
  CHECK(!m.contains("name1"));
  CHECK_EQ(m.get("name1"), nullopt);
  CHECK_EQ(c.get("name1"), nullopt);

  m.set("name1", "value1");
  CHECK(!m.empty());
  CHECK_EQ(m.size(), 1);
  CHECK(m.contains("name1"));
  CHECK(!m.contains("name2"));
  CHECK_EQ(m.get("name1"), "value1");
  CHECK_EQ(c.get("name1"), "value1");

  m.set("name2", "value2");
  CHECK(!m.empty());
  CHECK_EQ(m.size(), 2);
  CHECK(m.contains("name1"));
  CHECK(m.contains("name2"));
  CHECK_EQ(m.get("name2"), "value2");
  CHECK_EQ(c.get("name2"), "value2");

  CHECK_EQ(m.erase("name3"), nullopt);

  CHECK_EQ(m.erase("name1"), "value1");
  CHECK(!m.empty());
  CHECK_EQ(m.size(), 1);
  CHECK_EQ(m.get("name1"), nullopt);
  CHECK_EQ(c.get("name1"), nullopt);

  m.clear();
  CHECK(m.empty());
}

TEST_SUITE_END();
