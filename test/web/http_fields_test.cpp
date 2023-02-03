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
  http_fields fields;
  const auto& cfields = fields;
  fields.set(http::field::content_type,
             http::fields::content_type::plaintext());
  CHECK_EQ(fields.get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(fields.get("Content-Type"), http::fields::content_type::plaintext());
  CHECK_EQ(fields.get("content-type"), http::fields::content_type::plaintext());
  CHECK_EQ(cfields.get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(cfields.get("Content-Type"),
           http::fields::content_type::plaintext());
  CHECK_EQ(cfields.get("content-type"),
           http::fields::content_type::plaintext());
  CHECK_EQ(fields.at(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(fields.at("Content-Type"), http::fields::content_type::plaintext());
  CHECK_EQ(fields.at("content-type"), http::fields::content_type::plaintext());
  CHECK_EQ(cfields.at(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(cfields.at("Content-Type"), http::fields::content_type::plaintext());
  CHECK_EQ(cfields.at("content-type"), http::fields::content_type::plaintext());
  CHECK_EQ(fields.operator[](http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(fields.operator[]("Content-Type"),
           http::fields::content_type::plaintext());
  CHECK_EQ(fields.operator[]("content-type"),
           http::fields::content_type::plaintext());
  CHECK(fields.contains(http::field::content_type));
  CHECK(fields.contains("Content-Type"));
  CHECK(fields.contains("content-type"));

  fields.set("accept-encoding", "gzip");
  CHECK_EQ(fields.get(http::field::accept_encoding), "gzip");
  CHECK_EQ(fields.get("Accept-Encoding"), "gzip");
  CHECK_EQ(fields.get("accept-encoding"), "gzip");
  CHECK_EQ(cfields.get(http::field::accept_encoding), "gzip");
  CHECK_EQ(cfields.get("Accept-Encoding"), "gzip");
  CHECK_EQ(cfields.get("accept-encoding"), "gzip");
  CHECK_EQ(fields.at(http::field::accept_encoding), "gzip");
  CHECK_EQ(fields.at("Accept-Encoding"), "gzip");
  CHECK_EQ(fields.at("accept-encoding"), "gzip");
  CHECK_EQ(cfields.at(http::field::accept_encoding), "gzip");
  CHECK_EQ(cfields.at("Accept-Encoding"), "gzip");
  CHECK_EQ(cfields.at("accept-encoding"), "gzip");
  CHECK_EQ(fields.operator[](http::field::accept_encoding), "gzip");
  CHECK_EQ(fields.operator[]("Accept-Encoding"), "gzip");
  CHECK_EQ(fields.operator[]("accept-encoding"), "gzip");
  CHECK(fields.contains(http::field::accept_encoding));
  CHECK(fields.contains("Accept-Encoding"));
  CHECK(fields.contains("accept-encoding"));

  CHECK_EQ(fields.get(http::field::connection), nullopt);
  CHECK_EQ(fields.get("Connection"), nullopt);
  CHECK_EQ(fields.get("connection"), nullopt);
  CHECK_EQ(cfields.get(http::field::connection), nullopt);
  CHECK_EQ(cfields.get("Connection"), nullopt);
  CHECK_EQ(cfields.get("connection"), nullopt);
  CHECK_THROWS_AS(fields.at(http::field::connection), std::out_of_range);
  CHECK_THROWS_AS(fields.at("Connection"), std::out_of_range);
  CHECK_THROWS_AS(fields.at("connection"), std::out_of_range);
  CHECK_THROWS_AS(cfields.at(http::field::connection), std::out_of_range);
  CHECK_THROWS_AS(cfields.at("Connection"), std::out_of_range);
  CHECK_THROWS_AS(cfields.at("connection"), std::out_of_range);
  CHECK(!fields.contains(http::field::connection));
  CHECK(!fields.contains("Connection"));
  CHECK(!fields.contains("connection"));

  CHECK_EQ(fields.erase(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK(!fields.contains(http::field::content_type));
  CHECK_EQ(fields.erase("accept-encoding"), "gzip");
  CHECK(!fields.contains(http::field::accept_encoding));
}

TEST_SUITE_END();
