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

#include <chrono>

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
  using clock_t = std::chrono::system_clock;

  http_request() = default;

  http_request(net::ip::tcp::endpoint local_endpoint,
               net::ip::tcp::endpoint remote_endpoint,
               http_route route,
               std::string path,
               http::verb method,
               query_map query,
               http_header header,
               std::string body)
      : http_message(std::move(header), std::move(body))
      , local_endpoint_(local_endpoint)
      , remote_endpoint_(remote_endpoint)
      , route_(std::move(route))
      , path_(std::move(path))
      , method_(method)
      , query_(std::move(query))
  {
  }

  const net::ip::tcp::endpoint& local_endpoint() const noexcept
  {
    return local_endpoint_;
  }

  const net::ip::tcp::endpoint& remote_endpoint() const noexcept
  {
    return remote_endpoint_;
  }

  const http_route& route() const noexcept
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

  const std::string& path() const noexcept
  {
    return path_;
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

  template <typename T = json::value>
  http_request& set_json(const T& obj)
  {
    base().set_json(obj);
    return *this;
  }

  clock_t::time_point start_time() const noexcept
  {
    return start_time_;
  }

  clock_t::duration time_since_start() const noexcept
  {
    return clock_t::now() - start_time();
  }

private:
  net::ip::tcp::endpoint local_endpoint_;
  net::ip::tcp::endpoint remote_endpoint_;
  http_route route_;
  std::string path_;
  http::verb method_;
  query_map query_;
  clock_t::time_point start_time_ = clock_t::now();
};

FITORIA_NAMESPACE_END
