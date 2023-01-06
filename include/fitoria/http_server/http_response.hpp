//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/http_server/http_message.hpp>

FITORIA_NAMESPACE_BEGIN

class http_response : public http_message {
  using base_type = http_message;
  using native_response_t = http::response<http::string_body>;

  base_type& base() noexcept
  {
    return static_cast<base_type&>(*this);
  }

  const base_type& base() const noexcept
  {
    return static_cast<const base_type&>(*this);
  }

public:
  http_response(http::status status)
      : status_(status)
  {
  }

  http::status status() const noexcept
  {
    return status_;
  }

  http_response& set_status(http::status status) noexcept
  {
    status_ = status;
    return *this;
  }

  http_response& set_header(std::string name, std::string value)
  {
    headers().set(std::move(name), std::move(value));
    return *this;
  }

  http_response& set_header(http::field name, std::string value)
  {
    headers().set(name, std::move(value));
    return *this;
  }

  http_response& set_body(std::string body)
  {
    base().set_body(std::move(body));
    return *this;
  }

  http_response& set_json(const json::value& jv)
  {
    base().set_json(jv);
    return *this;
  }

  template <typename T>
  http_response& set_json(const T& obj)
  {
    base().set_json(obj);
    return *this;
  }

  operator native_response_t() const&
  {
    native_response_t res;
    res.result(status_);
    for (auto&& header : headers()) {
      res.insert(header.first, header.second);
    }
    res.body() = body();

    return res;
  }

  operator native_response_t() &&
  {
    native_response_t res;
    res.result(status_);
    for (auto&& header : headers()) {
      res.insert(header.first, header.second);
    }
    res.body() = std::move(body());

    return res;
  }

private:
  http::status status_;
};

FITORIA_NAMESPACE_END
