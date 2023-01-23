//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/http_header.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("web.http_header");

TEST_CASE("basic")
{
  http_header header;
  const auto& cheader = header;
  header.set(http::field::content_type,
             http::fields::content_type::plaintext());
  CHECK_EQ(header.get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(header.get("Content-Type"), http::fields::content_type::plaintext());
  CHECK_EQ(header.get("content-type"), http::fields::content_type::plaintext());
  CHECK_EQ(cheader.get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(cheader.get("Content-Type"),
           http::fields::content_type::plaintext());
  CHECK_EQ(cheader.get("content-type"),
           http::fields::content_type::plaintext());
  CHECK_EQ(header.at(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(header.at("Content-Type"), http::fields::content_type::plaintext());
  CHECK_EQ(header.at("content-type"), http::fields::content_type::plaintext());
  CHECK_EQ(cheader.at(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(cheader.at("Content-Type"), http::fields::content_type::plaintext());
  CHECK_EQ(cheader.at("content-type"), http::fields::content_type::plaintext());
  CHECK_EQ(header.operator[](http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(header.operator[]("Content-Type"),
           http::fields::content_type::plaintext());
  CHECK_EQ(header.operator[]("content-type"),
           http::fields::content_type::plaintext());
  CHECK(header.contains(http::field::content_type));
  CHECK(header.contains("Content-Type"));
  CHECK(header.contains("content-type"));

  header.set("accept-encoding", "gzip");
  CHECK_EQ(header.get(http::field::accept_encoding), "gzip");
  CHECK_EQ(header.get("Accept-Encoding"), "gzip");
  CHECK_EQ(header.get("accept-encoding"), "gzip");
  CHECK_EQ(cheader.get(http::field::accept_encoding), "gzip");
  CHECK_EQ(cheader.get("Accept-Encoding"), "gzip");
  CHECK_EQ(cheader.get("accept-encoding"), "gzip");
  CHECK_EQ(header.at(http::field::accept_encoding), "gzip");
  CHECK_EQ(header.at("Accept-Encoding"), "gzip");
  CHECK_EQ(header.at("accept-encoding"), "gzip");
  CHECK_EQ(cheader.at(http::field::accept_encoding), "gzip");
  CHECK_EQ(cheader.at("Accept-Encoding"), "gzip");
  CHECK_EQ(cheader.at("accept-encoding"), "gzip");
  CHECK_EQ(header.operator[](http::field::accept_encoding), "gzip");
  CHECK_EQ(header.operator[]("Accept-Encoding"), "gzip");
  CHECK_EQ(header.operator[]("accept-encoding"), "gzip");
  CHECK(header.contains(http::field::accept_encoding));
  CHECK(header.contains("Accept-Encoding"));
  CHECK(header.contains("accept-encoding"));

  CHECK_EQ(header.get(http::field::connection), nullopt);
  CHECK_EQ(header.get("Connection"), nullopt);
  CHECK_EQ(header.get("connection"), nullopt);
  CHECK_EQ(cheader.get(http::field::connection), nullopt);
  CHECK_EQ(cheader.get("Connection"), nullopt);
  CHECK_EQ(cheader.get("connection"), nullopt);
  CHECK_THROWS_AS(header.at(http::field::connection), std::out_of_range);
  CHECK_THROWS_AS(header.at("Connection"), std::out_of_range);
  CHECK_THROWS_AS(header.at("connection"), std::out_of_range);
  CHECK_THROWS_AS(cheader.at(http::field::connection), std::out_of_range);
  CHECK_THROWS_AS(cheader.at("Connection"), std::out_of_range);
  CHECK_THROWS_AS(cheader.at("connection"), std::out_of_range);
  CHECK(!header.contains(http::field::connection));
  CHECK(!header.contains("Connection"));
  CHECK(!header.contains("connection"));

  header.erase(http::field::content_type);
  CHECK(!header.contains(http::field::content_type));
  header.erase("accept-encoding");
  CHECK(!header.contains(http::field::accept_encoding));
}

TEST_SUITE_END();
