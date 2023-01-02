//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/http_server/http_header.hpp>

#include <string>

FITORIA_NAMESPACE_BEGIN

class http_response {
public:
  http_response(http::status status)
      : status_(status)
  {
  }

  http_header& headers() noexcept
  {
    return header_;
  }

  const http_header& headers() const noexcept
  {
    return header_;
  }

  http_response& set_header(std::string name, std::string value)
  {
    header_.set(std::move(name), std::move(value));
    return *this;
  }

  http_response& set_header(http::field name, std::string value)
  {
    header_.set(name, std::move(value));
    return *this;
  }

  http_response& set_body(std::string body)
  {
    body_ = std::move(body);
    return *this;
  }

  http_response& set_json(const json::value& jv)
  {
    header_.set(http::field::content_type, "application/json");
    body_ = json::serialize(jv);
    return *this;
  }

  template <typename T>
  http_response& set_json(const T& obj)
  {
    header_.set(http::field::content_type, "application/json");
    body_ = json::serialize(json::value_from(obj));
    return *this;
  }

  operator http::response<http::string_body>() const&
  {
    http::response<http::string_body> res;
    res.result(status_);
    for (auto&& header : header_) {
      res.insert(header.first, header.second);
    }
    res.body() = body_;

    return res;
  }

  operator http::response<http::string_body>() &&
  {
    http::response<http::string_body> res;
    res.result(status_);
    for (auto&& header : header_) {
      res.insert(header.first, header.second);
    }
    res.body() = std::move(body_);

    return res;
  }

private:
  http::status status_;
  http_header header_;
  std::string body_;
};

FITORIA_NAMESPACE_END
