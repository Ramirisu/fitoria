//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/http_server/http_message.hpp>
#include <fitoria/http_server/http_route.hpp>

FITORIA_NAMESPACE_BEGIN

class http_request : public http_message {
  using base_type = http_message;

  base_type& base() noexcept
  {
    return static_cast<base_type&>(*this);
  }

  const base_type& base() const noexcept
  {
    return static_cast<const base_type&>(*this);
  }

public:
  explicit http_request(net::ip::tcp::endpoint remote_endpoint,
                        http_route route,
                        std::string path,
                        http::verb method,
                        query_map query,
                        http_header header,
                        std::string body)
      : http_message(std::move(header), std::move(body))
      , remote_endpoint_(remote_endpoint)
      , route_(std::move(route))
      , path_(std::move(path))
      , method_(method)
      , query_(std::move(query))
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

  http_request& set_path(std::string path) noexcept
  {
    path_ = std::move(path);
    return *this;
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

  http_request& set_header(std::string name, std::string value)
  {
    headers().set(std::move(name), std::move(value));
    return *this;
  }

  http_request& set_header(http::field name, std::string value)
  {
    headers().set(name, std::move(value));
    return *this;
  }

  http_request& set_body(std::string body)
  {
    base().set_body(std::move(body));
    return *this;
  }

  http_request& set_json(const json::value& jv)
  {
    base().set_json(jv);
    return *this;
  }

  template <typename T>
  http_request& set_json(const T& obj)
  {
    base().set_json(obj);
    return *this;
  }

private:
  net::ip::tcp::endpoint remote_endpoint_;
  http_route route_;
  std::string path_;
  http::verb method_;
  query_map query_;
};

FITORIA_NAMESPACE_END
