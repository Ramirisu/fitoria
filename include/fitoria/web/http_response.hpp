//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_RESPONSE_HPP
#define FITORIA_WEB_HTTP_RESPONSE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>

#include <fitoria/web/http.hpp>
#include <fitoria/web/http_fields.hpp>
#include <fitoria/web/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_response {
  using native_response_t = http::detail::response<http::detail::string_body>;

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
      insert_field(field.name(), field.value());
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

  http_fields& fields() noexcept
  {
    return fields_;
  }

  const http_fields& fields() const noexcept
  {
    return fields_;
  }

  http_response& set_field(std::string name, std::string_view value)
  {
    fields_.set(std::move(name), value);
    return *this;
  }

  http_response& set_field(http::field name, std::string_view value)
  {
    fields_.set(name, value);
    return *this;
  }

  http_response& insert_field(std::string name, std::string_view value)
  {
    fields_.insert(std::move(name), value);
    return *this;
  }

  http_response& insert_field(http::field name, std::string_view value)
  {
    fields_.insert(name, value);
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

  http_response& set_body(std::string body)
  {
    body_ = std::move(body);
    return *this;
  }

  template <typename T = json::value>
  http_response& set_json(const T& obj)
  {
    if constexpr (std::is_same_v<T, json::value>) {
      fields_.set(http::field::content_type,
                  http::fields::content_type::json());
      body_ = json::serialize(obj);
    } else {
      set_json(json::value_from(obj));
    }
    return *this;
  }

  operator native_response_t() const&
  {
    native_response_t res;
    res.result(status_code_.value());
    for (auto&& fields : fields_) {
      res.insert(fields.first, fields.second);
    }
    res.body() = body();

    return res;
  }

  operator native_response_t() &&
  {
    native_response_t res;
    res.result(status_code_.value());
    for (auto&& fields : fields_) {
      res.insert(fields.first, fields.second);
    }
    res.body() = std::move(body());

    return res;
  }

private:
  http::status_code status_code_ = http::status::ok;
  http_fields fields_;
  std::string body_;
};

}

FITORIA_NAMESPACE_END

#endif
