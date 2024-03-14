//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/route_params.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.route_params]");

TEST_CASE("path")
{
  route_params params("/api/v1/users/{user}", {});
  CHECK_EQ(params.path(), "/api/v1/users/{user}");
}

TEST_CASE("misc")
{
  using value_type = route_params::value_type;

  route_params params(
      "", { { "key0", "value0" }, { "key1", "value1" }, { "key2", "value2" } });

  CHECK_EQ(params.keys(), route_params::keys_type { "key0", "key1", "key2" });

  CHECK(!params.empty());

  CHECK_EQ(params.size(), 3);

  CHECK(params.contains("key0"));
  CHECK(params.contains("key1"));
  CHECK(params.contains("key2"));
  CHECK(!params.contains("key3"));

  CHECK_EQ(params.get("key0"), "value0");
  CHECK_EQ(params.get("key1"), "value1");
  CHECK_EQ(params.get("key2"), "value2");
  CHECK_EQ(params.get("key3"), nullopt);

  CHECK_EQ(params.get(0), "value0");
  CHECK_EQ(params.get(1), "value1");
  CHECK_EQ(params.get(2), "value2");
  CHECK_EQ(params.get(3), nullopt);

  CHECK_EQ(params.at("key0"), "value0");
  CHECK_EQ(params.at("key1"), "value1");
  CHECK_EQ(params.at("key2"), "value2");
  CHECK_THROWS_AS(params.at("key3"), std::out_of_range);

  CHECK_EQ(params.at(0), "value0");
  CHECK_EQ(params.at(1), "value1");
  CHECK_EQ(params.at(2), "value2");
  CHECK_THROWS_AS(params.at(3), std::out_of_range);

  CHECK_EQ(*params.find("key0"), value_type { "key0", "value0" });
  CHECK_EQ(*params.find("key1"), value_type { "key1", "value1" });
  CHECK_EQ(*params.find("key2"), value_type { "key2", "value2" });
}

TEST_CASE("iterator")
{
  using value_type = route_params::value_type;

  SUBCASE("begin")
  {
    route_params params(
        "",
        { { "key0", "value0" }, { "key1", "value1" }, { "key2", "value2" } });

    auto it = params.begin();
    CHECK_EQ(it->first, "key0");
    CHECK_EQ(it->second, "value0");
    CHECK_EQ(*it++, value_type { "key0", "value0" });
    CHECK_EQ(it->first, "key1");
    CHECK_EQ(it->second, "value1");
    CHECK_EQ(*it++, value_type { "key1", "value1" });
    CHECK_EQ(it->first, "key2");
    CHECK_EQ(it->second, "value2");
    CHECK_EQ(*it++, value_type { "key2", "value2" });
    CHECK_EQ(it--, params.end());
    CHECK_EQ(*it--, value_type { "key2", "value2" });
    CHECK_EQ(*it--, value_type { "key1", "value1" });
    CHECK_EQ(*it, value_type { "key0", "value0" });
    CHECK_EQ(it, params.begin());
  }
  SUBCASE("begin const")
  {
    const route_params params(
        "",
        { { "key0", "value0" }, { "key1", "value1" }, { "key2", "value2" } });

    auto it = params.begin();
    CHECK_EQ(it->first, "key0");
    CHECK_EQ(it->second, "value0");
    CHECK_EQ(*it++, value_type { "key0", "value0" });
    CHECK_EQ(it->first, "key1");
    CHECK_EQ(it->second, "value1");
    CHECK_EQ(*it++, value_type { "key1", "value1" });
    CHECK_EQ(it->first, "key2");
    CHECK_EQ(it->second, "value2");
    CHECK_EQ(*it++, value_type { "key2", "value2" });
    CHECK_EQ(it--, params.end());
    CHECK_EQ(*it--, value_type { "key2", "value2" });
    CHECK_EQ(*it--, value_type { "key1", "value1" });
    CHECK_EQ(*it, value_type { "key0", "value0" });
    CHECK_EQ(it, params.begin());
  }
  SUBCASE("cbegin const")
  {
    route_params params(
        "",
        { { "key0", "value0" }, { "key1", "value1" }, { "key2", "value2" } });

    auto it = params.cbegin();
    CHECK_EQ(it->first, "key0");
    CHECK_EQ(it->second, "value0");
    CHECK_EQ(*it++, value_type { "key0", "value0" });
    CHECK_EQ(it->first, "key1");
    CHECK_EQ(it->second, "value1");
    CHECK_EQ(*it++, value_type { "key1", "value1" });
    CHECK_EQ(it->first, "key2");
    CHECK_EQ(it->second, "value2");
    CHECK_EQ(*it++, value_type { "key2", "value2" });
    CHECK_EQ(it--, params.cend());
    CHECK_EQ(*it--, value_type { "key2", "value2" });
    CHECK_EQ(*it--, value_type { "key1", "value1" });
    CHECK_EQ(*it, value_type { "key0", "value0" });
    CHECK_EQ(it, params.cbegin());
  }
}

TEST_SUITE_END();
