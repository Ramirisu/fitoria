//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/http.hpp>
#include <fitoria/core/json.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/http_server/http_header.hpp>
#include <fitoria/http_server/http_route.hpp>
#include <fitoria/http_server/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

class http_request {
public:
  explicit http_request(net::ip::tcp::endpoint remote_endpoint,
                        http_route route,
                        std::string path,
                        http::verb method,
                        std::string query_string,
                        query_map query,
                        http_header header,
                        std::string body)
      : remote_endpoint_(remote_endpoint)
      , route_(std::move(route))
      , path_(std::move(path))
      , method_(method)
      , query_string_(std::move(query_string))
      , query_(std::move(query))
      , header_(std::move(header))
      , body_(std::move(body))
  {
  }

  const net::ip::tcp::endpoint& remote_endpoint() const noexcept
  {
    return remote_endpoint_;
  }

  http_route& route() noexcept
  {
    return route_;
  }

  const http_route& route() const noexcept
  {
    return route_;
  }

  operator http_route&() noexcept
  {
    return route_;
  }

  operator const http_route&() const noexcept
  {
    return route_;
  }

  http::verb method() const noexcept
  {
    return method_;
  }

  std::string& path() noexcept
  {
    return path_;
  }

  const std::string& path() const noexcept
  {
    return path_;
  }

  std::string& query_string() noexcept
  {
    return query_string_;
  }

  const std::string& query_string() const noexcept
  {
    return query_string_;
  }

  query_map& query() noexcept
  {
    return query_;
  }

  const query_map& query() const noexcept
  {
    return query_;
  }

  operator query_map&() noexcept
  {
    return query_;
  }

  operator const query_map&() const noexcept
  {
    return query_;
  }

  http_header& headers() noexcept
  {
    return header_;
  }

  const http_header& headers() const noexcept
  {
    return header_;
  }

  std::string& body() noexcept
  {
    return body_;
  }

  const std::string& body() const noexcept
  {
    return body_;
  }

  expected<json::value, error_code> parse_json() const
  {
    if (headers().get(http::field::content_type) != "application/json") {
      return unexpected { make_error_code(error::unexpected_content_type) };
    }

    json::error_code ec;
    auto jv = json::parse(body(), ec);
    if (ec) {
      return unexpected { make_error_code(error::invalid_json_format) };
    }

    return jv;
  }

  template <typename T>
  expected<T, error_code> parse_json_to() const
  {
    return parse_json().transform(
        [](auto&& jv) { return json::value_to<T>(jv); });
  }

  expected<query_map, error_code> parse_post_form() const
  {
    if (headers().get(http::field::content_type)
        != "application/x-www-form-urlencoded") {
      return unexpected { make_error_code(error::unexpected_content_type) };
    }

    auto res = urls::parse_query(body());
    if (!res) {
      return unexpected { make_error_code(error::invalid_form_format) };
    }

    auto params = static_cast<urls::params_view>(res.value());

    query_map map;
    for (auto it = params.begin(); it != params.end(); ++it) {
      auto kv = *it;
      if (kv.has_value) {
        map.set(kv.key, kv.value);
      }
    }

    return map;
  }

private:
  net::ip::tcp::endpoint remote_endpoint_;
  http_route route_;
  std::string path_;
  http::verb method_;
  std::string query_string_;
  query_map query_;
  http_header header_;
  std::string body_;
};

FITORIA_NAMESPACE_END
