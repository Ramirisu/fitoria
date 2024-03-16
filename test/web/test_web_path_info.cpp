//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/web/path_info.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.path_info]");

TEST_CASE("match_pattern and match_path")
{
  path_info path_info("/api/v1/users/{user}", "/api/v1/users/ramirisu", {});
  CHECK_EQ(path_info.match_pattern(), "/api/v1/users/{user}");
  CHECK_EQ(path_info.match_path(), "/api/v1/users/ramirisu");
}

TEST_CASE("misc")
{
  using value_type = path_info::value_type;

  path_info path_info(
      "",
      "",
      { { "key0", "value0" }, { "key1", "value1" }, { "key2", "value2" } });

  CHECK_EQ(path_info.keys(), path_info::keys_type { "key0", "key1", "key2" });

  CHECK(!path_info.empty());

  CHECK_EQ(path_info.size(), 3);

  CHECK(path_info.contains("key0"));
  CHECK(path_info.contains("key1"));
  CHECK(path_info.contains("key2"));
  CHECK(!path_info.contains("key3"));

  CHECK_EQ(path_info.get("key0"), "value0");
  CHECK_EQ(path_info.get("key1"), "value1");
  CHECK_EQ(path_info.get("key2"), "value2");
  CHECK_EQ(path_info.get("key3"), nullopt);

  CHECK_EQ(path_info.get(0), "value0");
  CHECK_EQ(path_info.get(1), "value1");
  CHECK_EQ(path_info.get(2), "value2");
  CHECK_EQ(path_info.get(3), nullopt);

  CHECK_EQ(path_info.at("key0"), "value0");
  CHECK_EQ(path_info.at("key1"), "value1");
  CHECK_EQ(path_info.at("key2"), "value2");
  CHECK_THROWS_AS(path_info.at("key3"), std::out_of_range);

  CHECK_EQ(path_info.at(0), "value0");
  CHECK_EQ(path_info.at(1), "value1");
  CHECK_EQ(path_info.at(2), "value2");
  CHECK_THROWS_AS(path_info.at(3), std::out_of_range);

  CHECK_EQ(*path_info.find("key0"), value_type { "key0", "value0" });
  CHECK_EQ(*path_info.find("key1"), value_type { "key1", "value1" });
  CHECK_EQ(*path_info.find("key2"), value_type { "key2", "value2" });
}

TEST_CASE("iterator")
{
  using value_type = path_info::value_type;

  SUBCASE("begin")
  {
    path_info path_info(
        "",
        "",
        { { "key0", "value0" }, { "key1", "value1" }, { "key2", "value2" } });

    auto it = path_info.begin();
    CHECK_EQ(it->first, "key0");
    CHECK_EQ(it->second, "value0");
    CHECK_EQ(*it++, value_type { "key0", "value0" });
    CHECK_EQ(it->first, "key1");
    CHECK_EQ(it->second, "value1");
    CHECK_EQ(*it++, value_type { "key1", "value1" });
    CHECK_EQ(it->first, "key2");
    CHECK_EQ(it->second, "value2");
    CHECK_EQ(*it++, value_type { "key2", "value2" });
    CHECK_EQ(it--, path_info.end());
    CHECK_EQ(*it--, value_type { "key2", "value2" });
    CHECK_EQ(*it--, value_type { "key1", "value1" });
    CHECK_EQ(*it, value_type { "key0", "value0" });
    CHECK_EQ(it, path_info.begin());
  }
  SUBCASE("begin const")
  {
    const path_info path_info(
        "",
        "",
        { { "key0", "value0" }, { "key1", "value1" }, { "key2", "value2" } });

    auto it = path_info.begin();
    CHECK_EQ(it->first, "key0");
    CHECK_EQ(it->second, "value0");
    CHECK_EQ(*it++, value_type { "key0", "value0" });
    CHECK_EQ(it->first, "key1");
    CHECK_EQ(it->second, "value1");
    CHECK_EQ(*it++, value_type { "key1", "value1" });
    CHECK_EQ(it->first, "key2");
    CHECK_EQ(it->second, "value2");
    CHECK_EQ(*it++, value_type { "key2", "value2" });
    CHECK_EQ(it--, path_info.end());
    CHECK_EQ(*it--, value_type { "key2", "value2" });
    CHECK_EQ(*it--, value_type { "key1", "value1" });
    CHECK_EQ(*it, value_type { "key0", "value0" });
    CHECK_EQ(it, path_info.begin());
  }
  SUBCASE("cbegin const")
  {
    path_info path_info(
        "",
        "",
        { { "key0", "value0" }, { "key1", "value1" }, { "key2", "value2" } });

    auto it = path_info.cbegin();
    CHECK_EQ(it->first, "key0");
    CHECK_EQ(it->second, "value0");
    CHECK_EQ(*it++, value_type { "key0", "value0" });
    CHECK_EQ(it->first, "key1");
    CHECK_EQ(it->second, "value1");
    CHECK_EQ(*it++, value_type { "key1", "value1" });
    CHECK_EQ(it->first, "key2");
    CHECK_EQ(it->second, "value2");
    CHECK_EQ(*it++, value_type { "key2", "value2" });
    CHECK_EQ(it--, path_info.cend());
    CHECK_EQ(*it--, value_type { "key2", "value2" });
    CHECK_EQ(*it--, value_type { "key1", "value1" });
    CHECK_EQ(*it, value_type { "key0", "value0" });
    CHECK_EQ(it, path_info.cbegin());
  }
}

TEST_SUITE_END();
