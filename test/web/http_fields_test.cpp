//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/http_fields.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("web.http_fields");

TEST_CASE("basic")
{
  http_fields fs;
  const auto& cfs = fs;

  fs.set(http::field::content_type, http::fields::content_type::plaintext());
  CHECK_EQ(fs.get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(fs.get("Content-Type"), http::fields::content_type::plaintext());
  CHECK_EQ(fs.get("content-type"), http::fields::content_type::plaintext());
  CHECK_EQ(cfs.get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(cfs.get("Content-Type"), http::fields::content_type::plaintext());
  CHECK_EQ(cfs.get("content-type"), http::fields::content_type::plaintext());
  CHECK(fs.contains(http::field::content_type));
  CHECK(fs.contains("Content-Type"));
  CHECK(fs.contains("content-type"));

  fs.set("accept-encoding", "gzip");
  CHECK_EQ(fs.get(http::field::accept_encoding), "gzip");
  CHECK_EQ(fs.get("Accept-Encoding"), "gzip");
  CHECK_EQ(fs.get("accept-encoding"), "gzip");
  CHECK_EQ(cfs.get(http::field::accept_encoding), "gzip");
  CHECK_EQ(cfs.get("Accept-Encoding"), "gzip");
  CHECK_EQ(cfs.get("accept-encoding"), "gzip");
  CHECK(fs.contains(http::field::accept_encoding));
  CHECK(fs.contains("Accept-Encoding"));
  CHECK(fs.contains("accept-encoding"));

  CHECK_EQ(fs.get(http::field::connection), nullopt);
  CHECK_EQ(fs.get("Connection"), nullopt);
  CHECK_EQ(fs.get("connection"), nullopt);
  CHECK_EQ(cfs.get(http::field::connection), nullopt);
  CHECK_EQ(cfs.get("Connection"), nullopt);
  CHECK_EQ(cfs.get("connection"), nullopt);
  CHECK(!fs.contains(http::field::connection));
  CHECK(!fs.contains("Connection"));
  CHECK(!fs.contains("connection"));

  CHECK_EQ(fs.erase(http::field::content_type), 1);
  CHECK(!fs.contains(http::field::content_type));
  CHECK_EQ(fs.erase("accept-encoding"), 1);
  CHECK(!fs.contains(http::field::accept_encoding));
}

TEST_CASE("insert")
{
  http_fields fs;
  const auto& cfs = fs;

  fs.insert(http::field::content_type, http::fields::content_type::plaintext());
  CHECK_EQ(fs.get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(fs.get("Content-Type"), http::fields::content_type::plaintext());
  CHECK_EQ(fs.get("content-type"), http::fields::content_type::plaintext());
  CHECK_EQ(cfs.get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(cfs.get("Content-Type"), http::fields::content_type::plaintext());
  CHECK_EQ(cfs.get("content-type"), http::fields::content_type::plaintext());
  CHECK(fs.contains(http::field::content_type));
  CHECK(fs.contains("Content-Type"));
  CHECK(fs.contains("content-type"));

  fs.insert(http::field::content_type, http::fields::content_type::json());
  CHECK_EQ(fs.size(), 2);
  {
    const auto exp
        = std::set<std::string_view> { http::fields::content_type::plaintext(),
                                       http::fields::content_type::json() };
    auto iters = fs.equal_range(http::field::content_type);
    int match_count = 0;
    for (auto iter = iters.first; iter != iters.second; ++iter) {
      if (exp.contains(iter->second)) {
        ++match_count;
      }
    }
    CHECK_EQ(match_count, 2);
  }

  fs.set(http::field::content_type,
         http::fields::content_type::form_urlencoded());
  CHECK_EQ(fs.size(), 1);
}

TEST_SUITE_END();
