//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
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

#include <fitoria/http_server/error.hpp>
#include <fitoria/http_server/http_header.hpp>
#include <fitoria/http_server/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

class http_message {
public:
  http_message() = default;

  http_message(http_header header, std::string body)
      : header_(std::move(header))
      , body_(std::move(body))
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

  http_message& set_header(std::string name, std::string value)
  {
    header_.set(std::move(name), std::move(value));
    return *this;
  }

  http_message& set_header(http::field name, std::string value)
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

  template <typename T = json::value>
  expected<T, error_code> body_as_json() const
  {
    if (headers().get(http::field::content_type) != "application/json") {
      return unexpected { make_error_code(error::unexpected_content_type) };
    }

    json::error_code ec;
    auto jv = json::parse(body(), ec);
    if (ec) {
      return unexpected { make_error_code(error::invalid_json_format) };
    }

    if constexpr (std::is_same_v<T, json::value>) {
      return jv;
    } else {
      return json::value_to<T>(jv);
    }
  }

  expected<query_map, error_code> body_as_post_form() const
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

  http_message& set_body(std::string body)
  {
    body_ = std::move(body);
    return *this;
  }

  template <typename T = json::value>
  http_message& set_json(const T& obj)
  {
    if constexpr (std::is_same_v<T, json::value>) {
      header_.set(http::field::content_type, "application/json");
      body_ = json::serialize(obj);
    } else {
      set_json(json::value_from(obj));
    }
    return *this;
  }

private:
  http_header header_;
  std::string body_;
};

FITORIA_NAMESPACE_END
