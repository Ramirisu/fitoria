//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/mime/params_view.hpp>

using namespace fitoria;
using namespace fitoria::mime;

TEST_SUITE_BEGIN("[fitoria.mime.params_view]");

TEST_CASE("default construction")
{
  auto p = params_view();
  CHECK(p.empty());
  CHECK_EQ(p.size(), 0);
  CHECK(!p.contains("charset"));
  CHECK_EQ(p.get("charset"), nullopt);
  CHECK_EQ(p.erase("charset"), nullopt);
}

TEST_CASE("contruction by init")
{
  auto p = params_view({ { "charset", "utf-8" } });
  CHECK(!p.empty());
  CHECK_EQ(p.size(), 1);
  CHECK(p.contains("charset"));
  CHECK_EQ(p.get("charset"), "utf-8");
  CHECK_EQ(p.at("charset"), "utf-8");
  CHECK_EQ(p["charset"], "utf-8");
  CHECK_EQ(p.get("any"), nullopt);
  CHECK_EQ(p.erase("any"), nullopt);

  const auto& cp = p;
  CHECK_EQ(cp.get("charset"), "utf-8");
  CHECK_EQ(cp.at("charset"), "utf-8");
  CHECK_EQ(cp.get("any"), nullopt);
}

TEST_CASE("copy/move")
{
  {
    auto s = params_view({ { "charset", "utf-8" } });
    params_view d(s);
    CHECK_EQ(d, params_view({ { "charset", "utf-8" } }));
  }
  {
    auto s = params_view({ { "charset", "utf-8" } });
    auto d = s;
    CHECK_EQ(d, params_view({ { "charset", "utf-8" } }));
  }
  {
    auto s = params_view({ { "charset", "utf-8" } });
    params_view d(std::move(s));
    CHECK_EQ(d, params_view({ { "charset", "utf-8" } }));
  }
  {
    auto s = params_view({ { "charset", "utf-8" } });
    auto d = std::move(s);
    CHECK_EQ(d, params_view({ { "charset", "utf-8" } }));
  }
}

TEST_CASE("set")
{
  auto p = params_view();
  p.set("charset", "utf-8");
  CHECK_EQ(p.get("charset"), "utf-8");

  CHECK_EQ(p.erase("charset"), "utf-8");
  CHECK_EQ(p.size(), 0);
  CHECK(!p.contains("charset"));
  CHECK_EQ(p.get("charset"), nullopt);
}

TEST_CASE("clear")
{
  auto p = params_view({ { "charset", "utf-8" } });
  p.clear();
  CHECK(p.empty());
  CHECK_EQ(p.size(), 0);
  CHECK(!p.contains("charset"));
}

TEST_CASE("iterator")
{
  {
    auto params = params_view({ { "charset", "utf-8" } });
    for (auto param : params) {
      CHECK_EQ(param.first, "charset");
      CHECK_EQ(param.second, "utf-8");
    }
  }
  {
    const auto params = params_view({ { "charset", "utf-8" } });
    for (auto param : params) {
      CHECK_EQ(param.first, "charset");
      CHECK_EQ(param.second, "utf-8");
    }
  }
  {
    auto params = params_view({ { "charset", "utf-8" } });
    auto it = params.cbegin();
    CHECK_EQ(it->first, "charset");
    CHECK_EQ(it->second, "utf-8");
    CHECK_EQ(++it, params.cend());
  }
}

TEST_SUITE_END();
