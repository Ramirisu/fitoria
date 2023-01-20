//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_REQUEST_HPP
#define FITORIA_WEB_HTTP_REQUEST_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/web/http.hpp>
#include <fitoria/web/http_header.hpp>
#include <fitoria/web/http_route.hpp>
#include <fitoria/web/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

class http_request {
public:
  http_request() = default;

  http_request(net::ip::tcp::endpoint local_endpoint,
               net::ip::tcp::endpoint remote_endpoint,
               http_route route,
               std::string path,
               http::verb method,
               query_map query,
               http_header header,
               std::string body)
      : local_endpoint_(local_endpoint)
      , remote_endpoint_(remote_endpoint)
      , route_(std::move(route))
      , path_(std::move(path))
      , method_(method)
      , query_(std::move(query))
      , header_(std::move(header))
      , body_(std::move(body))
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

  http_header& headers() noexcept
  {
    return header_;
  }

  const http_header& headers() const noexcept
  {
    return header_;
  }

  operator http_header&() noexcept
  {
    return header_;
  }

  operator const http_header&() const noexcept
  {
    return header_;
  }

  http_request& set_header(std::string name, std::string value)
  {
    header_.set(std::move(name), std::move(value));
    return *this;
  }

  http_request& set_header(http::field name, std::string value)
  {
    header_.set(name, std::move(value));
    return *this;
  }

  std::string& body() noexcept
  {
    return body_;
  }

  const std::string& body() const noexcept
  {
    return body_;
  }

  operator std::string&() noexcept
  {
    return body_;
  }

  operator const std::string&() const noexcept
  {
    return body_;
  }

  http_request& set_body(std::string body)
  {
    body_ = std::move(body);
    return *this;
  }

  template <typename T = json::value>
  http_request& set_json(const T& obj)
  {
    if constexpr (std::is_same_v<T, json::value>) {
      header_.set(http::field::content_type, "application/json");
      body_ = json::serialize(obj);
    } else {
      set_json(json::value_from(obj));
    }
    return *this;
  }

  http_request& prepare_payload()
  {
    if (!body().empty() || method_ == http::verb::options
        || method_ == http::verb::post || method_ == http::verb::put) {
      header_.set(http::field::content_length, std::to_string(body().size()));
    }
    return *this;
  }

private:
  net::ip::tcp::endpoint local_endpoint_;
  net::ip::tcp::endpoint remote_endpoint_;
  http_route route_;
  std::string path_;
  http::verb method_;
  query_map query_;
  http_header header_;
  std::string body_;
};

FITORIA_NAMESPACE_END

#endif
