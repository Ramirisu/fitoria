//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/web/http_message.hpp>

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
  http_response() = default;

  explicit http_response(http::status_code status_code)
      : status_code_(status_code)
  {
  }

  explicit http_response(native_response_t native)
      : status_code_(native.result())
  {
    for (auto& field : native) {
      set_header(field.name(), field.value());
    }
    set_body(std::move(native.body()));
  }

  http::status_code status_code() const noexcept
  {
    return status_code_;
  }

  http_response& set_status_code(http::status_code status_code) noexcept
  {
    status_code_ = status_code;
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

  template <typename T = json::value>
  http_response& set_json(const T& obj)
  {
    base().set_json(obj);
    return *this;
  }

  operator native_response_t() const&
  {
    native_response_t res;
    res.result(status_code_.value());
    for (auto&& header : headers()) {
      res.insert(header.first, header.second);
    }
    res.body() = body();

    return res;
  }

  operator native_response_t() &&
  {
    native_response_t res;
    res.result(status_code_.value());
    for (auto&& header : headers()) {
      res.insert(header.first, header.second);
    }
    res.body() = std::move(body());

    return res;
  }

private:
  http::status_code status_code_ = http::status::ok;
};

FITORIA_NAMESPACE_END
