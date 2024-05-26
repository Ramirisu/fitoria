//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/http/header.hpp>
#include <fitoria/mime.hpp>

using namespace fitoria;
using namespace fitoria::http;

TEST_SUITE_BEGIN("[fitoria.http.header]");

TEST_CASE("basic")
{
  header h;
  const auto& ch = h;

  h.set(http::field::content_type, mime::text_plain());
  CHECK_EQ(h.get(http::field::content_type), mime::text_plain());
  CHECK_EQ(h.get("Content-Type"), mime::text_plain());
  CHECK_EQ(h.get("content-type"), mime::text_plain());
  CHECK_EQ(ch.get(http::field::content_type), mime::text_plain());
  CHECK_EQ(ch.get("Content-Type"), mime::text_plain());
  CHECK_EQ(ch.get("content-type"), mime::text_plain());
  CHECK(h.contains(http::field::content_type));
  CHECK(h.contains("Content-Type"));
  CHECK(h.contains("content-type"));

  h.set("accept-encoding", "gzip");
  CHECK_EQ(h.get(http::field::accept_encoding), "gzip");
  CHECK_EQ(h.get("Accept-Encoding"), "gzip");
  CHECK_EQ(h.get("accept-encoding"), "gzip");
  CHECK_EQ(ch.get(http::field::accept_encoding), "gzip");
  CHECK_EQ(ch.get("Accept-Encoding"), "gzip");
  CHECK_EQ(ch.get("accept-encoding"), "gzip");
  CHECK(h.contains(http::field::accept_encoding));
  CHECK(h.contains("Accept-Encoding"));
  CHECK(h.contains("accept-encoding"));

  CHECK_EQ(h.get(http::field::connection), nullopt);
  CHECK_EQ(h.get("Connection"), nullopt);
  CHECK_EQ(h.get("connection"), nullopt);
  CHECK_EQ(ch.get(http::field::connection), nullopt);
  CHECK_EQ(ch.get("Connection"), nullopt);
  CHECK_EQ(ch.get("connection"), nullopt);
  CHECK(!h.contains(http::field::connection));
  CHECK(!h.contains("Connection"));
  CHECK(!h.contains("connection"));

  CHECK_EQ(h.erase(http::field::content_type), 1);
  CHECK(!h.contains(http::field::content_type));
  CHECK_EQ(h.erase("accept-encoding"), 1);
  CHECK(!h.contains(http::field::accept_encoding));
}

TEST_CASE("insert")
{
  header h;
  const auto& ch = h;

  h.insert(http::field::content_type, mime::text_plain());
  CHECK_EQ(h.get(http::field::content_type), mime::text_plain());
  CHECK_EQ(h.get("Content-Type"), mime::text_plain());
  CHECK_EQ(h.get("content-type"), mime::text_plain());
  CHECK_EQ(ch.get(http::field::content_type), mime::text_plain());
  CHECK_EQ(ch.get("Content-Type"), mime::text_plain());
  CHECK_EQ(ch.get("content-type"), mime::text_plain());
  CHECK(h.contains(http::field::content_type));
  CHECK(h.contains("Content-Type"));
  CHECK(h.contains("content-type"));

  h.insert(http::field::content_type, mime::application_json());
  CHECK_EQ(h.size(), 2);
  {
    const auto exp = std::set<std::string_view> { mime::text_plain(),
                                                  mime::application_json() };
    auto iters = h.equal_range(http::field::content_type);
    int match_count = 0;
    for (auto iter = iters.first; iter != iters.second; ++iter) {
      if (exp.contains(iter->value())) {
        ++match_count;
      }
    }
    CHECK_EQ(match_count, 2);
  }

  h.set(http::field::content_type, mime::application_www_form_urlencoded());
  CHECK_EQ(h.size(), 1);
}

TEST_SUITE_END();
