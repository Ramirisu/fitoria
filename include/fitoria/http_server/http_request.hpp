//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/http_server/http_handler_trait.hpp>
#include <fitoria/http_server/http_header.hpp>
#include <fitoria/http_server/http_route.hpp>
#include <fitoria/http_server/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

class http_request {
  using native_type = http::request<http::string_body>;

public:
  explicit http_request(net::ip::tcp::endpoint remote_endpoint,
                        http_route& route,
                        native_type& native,
                        std::string path,
                        std::string query_string,
                        query_map query)
      : remote_endpoint_(remote_endpoint)
      , route_(route)
      , native_(native)
      , path_(std::move(path))
      , query_string_(std::move(query_string))
      , query_(std::move(query))
  {
    for (auto it = native.begin(); it != native.end(); ++it) {
      header_.set(it->name_string(), it->value());
    }
  }

  net::ip::tcp::endpoint remote_endpoint() const noexcept
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
    return native_.method();
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
    return native_.body();
  }

  const std::string& body() const noexcept
  {
    return native_.body();
  }

private:
  net::ip::tcp::endpoint remote_endpoint_;
  http_route& route_;
  native_type& native_;
  std::string path_;
  std::string query_string_;
  query_map query_;
  http_header header_;
};

FITORIA_NAMESPACE_END
