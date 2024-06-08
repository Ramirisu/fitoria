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

  h.set("X-Trace-Id", "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(h.get("X-Trace-Id"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(h.get("X-TRACE-ID"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(h.get("x-trace-id"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(ch.get("X-Trace-Id"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(ch.get("X-TRACE-ID"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(ch.get("x-trace-id"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK(h.contains("X-Trace-Id"));
  CHECK(h.contains("X-TRACE-ID"));
  CHECK(h.contains("x-trace-id"));

  CHECK_EQ(h.get(http::field::connection), nullopt);
  CHECK_EQ(h.get("Connection"), nullopt);
  CHECK_EQ(h.get("connection"), nullopt);
  CHECK_EQ(ch.get(http::field::connection), nullopt);
  CHECK_EQ(ch.get("Connection"), nullopt);
  CHECK_EQ(ch.get("connection"), nullopt);
  CHECK(!h.contains(http::field::connection));
  CHECK(!h.contains("Connection"));
  CHECK(!h.contains("connection"));

  boost::beast::http::fields f;
  h.to_impl(f);

  CHECK_EQ(h.erase(http::field::content_type), 1);
  CHECK(!h.contains(http::field::content_type));
  CHECK_EQ(h.erase("accept-encoding"), 1);
  CHECK(!h.contains(http::field::accept_encoding));
  CHECK_EQ(h.erase("x-trace-id"), 1);
  CHECK(!h.contains("x-trace-id"));
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

  h.insert("X-Trace-Id", "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(h.get("X-Trace-Id"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(h.get("X-TRACE-ID"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(h.get("x-trace-id"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(ch.get("X-Trace-Id"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(ch.get("X-TRACE-ID"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK_EQ(ch.get("x-trace-id"), "66543f81-6ef3-44f0-aec5-3b952351d9c8");
  CHECK(h.contains("X-Trace-Id"));
  CHECK(h.contains("X-TRACE-ID"));
  CHECK(h.contains("x-trace-id"));

  h.insert(http::field::content_type, mime::application_json());
  CHECK_EQ(h.size(), 3);
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

  boost::beast::http::fields f;
  h.to_impl(f);

  h.set(http::field::content_type, mime::application_www_form_urlencoded());
  CHECK_EQ(h.size(), 2);
}

TEST_SUITE_END();
