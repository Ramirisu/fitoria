//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HTTP_RESPONSE_HPP
#define FITORIA_WEB_HTTP_RESPONSE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/detail/as_json.hpp>
#include <fitoria/web/error.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/http_fields.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_response {
public:
  http_response() = default;

  explicit http_response(http::status_code status_code)
      : status_code_(status_code)
  {
  }

  http_response(http::status_code status_code, http_fields fields)
      : status_code_(status_code)
      , fields_(std::move(fields))
  {
  }

  http_response(http::status_code status_code,
                http_fields fields,
                any_async_readable_stream body)
      : status_code_(status_code)
      , fields_(std::move(fields))
      , body_(std::move(body))
  {
  }

  http_response(const http_response&) = delete;

  http_response& operator=(const http_response&) = delete;

  http_response(http_response&&) = default;

  http_response& operator=(http_response&&) = default;

  http::status_code status_code() const noexcept
  {
    return status_code_;
  }

  http_response& set_status_code(http::status_code status_code) & noexcept
  {
    status_code_ = status_code;
    return *this;
  }

  http_response&& set_status_code(http::status_code status_code) && noexcept
  {
    status_code_ = status_code;
    return std::move(*this);
  }

  http_fields& fields() noexcept
  {
    return fields_;
  }

  const http_fields& fields() const noexcept
  {
    return fields_;
  }

  http_response& set_field(std::string name, std::string_view value) &
  {
    fields_.set(std::move(name), value);
    return *this;
  }

  http_response&& set_field(std::string name, std::string_view value) &&
  {
    fields_.set(std::move(name), value);
    return std::move(*this);
  }

  http_response& set_field(http::field name, std::string_view value) &
  {
    fields_.set(name, value);
    return *this;
  }

  http_response&& set_field(http::field name, std::string_view value) &&
  {
    fields_.set(name, value);
    return std::move(*this);
  }

  http_response& insert_field(std::string name, std::string_view value) &
  {
    fields_.insert(std::move(name), value);
    return *this;
  }

  http_response&& insert_field(std::string name, std::string_view value) &&
  {
    fields_.insert(std::move(name), value);
    return std::move(*this);
  }

  http_response& insert_field(http::field name, std::string_view value) &
  {
    fields_.insert(name, value);
    return *this;
  }

  http_response&& insert_field(http::field name, std::string_view value) &&
  {
    fields_.insert(name, value);
    return std::move(*this);
  }

  any_async_readable_stream& body() noexcept
  {
    return body_;
  }

  const any_async_readable_stream& body() const noexcept
  {
    return body_;
  }

  lazy<expected<std::string, net::error_code>> as_string()
  {
    if (auto str = co_await async_read_all_as<std::string>(body()); str) {
      co_return *str;
    }

    co_return std::string();
  }

  template <typename T = boost::json::value>
  lazy<expected<T, net::error_code>> as_json()
  {
    if (fields().get(http::field::content_type)
        != http::fields::content_type::json()) {
      co_return unexpected { make_error_code(error::unexpected_content_type) };
    }

    if (auto str = co_await async_read_all_as<std::string>(body()); str) {
      if (*str) {
        co_return detail::as_json<T>(**str);
      }

      co_return unexpected { (*str).error() };
    }

    co_return detail::as_json<T>("");
  }

  template <std::size_t N>
  http_response& set_body(std::span<const std::byte, N> bytes) &
  {
    body_ = any_async_readable_stream(async_readable_vector_stream(bytes));
    return *this;
  }

  template <std::size_t N>
  http_response&& set_body(std::span<const std::byte, N> bytes) &&
  {
    body_ = any_async_readable_stream(async_readable_vector_stream(bytes));
    return std::move(*this);
  }

  http_response& set_body(std::string_view sv) &
  {
    set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
    return *this;
  }

  http_response&& set_body(std::string_view sv) &&
  {
    set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
    return std::move(*this);
  }

  template <typename T = boost::json::value>
  http_response& set_json(const T& obj) &
  {
    if constexpr (std::is_same_v<T, boost::json::value>) {
      set_field(http::field::content_type, http::fields::content_type::json());
      auto s = boost::json::serialize(obj);
      set_body(std::as_bytes(std::span(s.begin(), s.end())));
    } else {
      set_json(boost::json::value_from(obj));
    }
    return *this;
  }

  template <typename T = boost::json::value>
  http_response&& set_json(const T& obj) &&
  {
    if constexpr (std::is_same_v<T, boost::json::value>) {
      set_field(http::field::content_type, http::fields::content_type::json());
      auto s = boost::json::serialize(obj);
      set_body(std::as_bytes(std::span(s.begin(), s.end())));
    } else {
      set_json(boost::json::value_from(obj));
    }
    return std::move(*this);
  }

  template <async_readable_stream AsyncReadableStream>
  http_response& set_stream(AsyncReadableStream&& stream) &
  {
    body_ = std::forward<AsyncReadableStream>(stream);
    return *this;
  }

  template <async_readable_stream AsyncReadableStream>
  http_response&& set_stream(AsyncReadableStream&& stream) &&
  {
    body_ = std::forward<AsyncReadableStream>(stream);
    return std::move(*this);
  }

private:
  http::status_code status_code_ = http::status::ok;
  http_fields fields_;
  any_async_readable_stream body_ { async_readable_vector_stream::eof() };
};

}

FITORIA_NAMESPACE_END

#endif
