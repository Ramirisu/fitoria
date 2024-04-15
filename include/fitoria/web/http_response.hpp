//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HTTP_RESPONSE_HPP
#define FITORIA_WEB_HTTP_RESPONSE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/json.hpp>

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/http_fields.hpp>

#include <span>

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

  optional<any_async_readable_stream&> body() noexcept
  {
    return optional<any_async_readable_stream&>(body_);
  }

  optional<const any_async_readable_stream&> body() const noexcept
  {
    return optional<const any_async_readable_stream&>(body_);
  }

  template <std::size_t N>
  http_response& set_body(std::span<const std::byte, N> bytes) &
  {
    body_.emplace(async_readable_vector_stream(bytes));
    return *this;
  }

  template <std::size_t N>
  http_response&& set_body(std::span<const std::byte, N> bytes) &&
  {
    set_body(bytes);
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

  http_response& set_json(const boost::json::value& jv) &
  {
    set_field(http::field::content_type, http::fields::content_type::json());
    auto str = boost::json::serialize(jv);
    set_body(std::as_bytes(std::span(str.begin(), str.end())));
    return *this;
  }

  http_response&& set_json(const boost::json::value& jv) &&
  {
    set_json(jv);
    return std::move(*this);
  }

  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  http_response& set_json(const T& obj) &
  {
    return set_json(boost::json::value_from(obj));
  }

  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  http_response&& set_json(const T& obj) &&
  {
    set_json(boost::json::value_from(obj));
    return std::move(*this);
  }

  template <async_readable_stream AsyncReadableStream>
  http_response& set_stream(AsyncReadableStream&& stream) &
  {
    body_.emplace(std::forward<AsyncReadableStream>(stream));
    return *this;
  }

  template <async_readable_stream AsyncReadableStream>
  http_response&& set_stream(AsyncReadableStream&& stream) &&
  {
    set_stream(std::forward<AsyncReadableStream>(stream));
    return std::move(*this);
  }

private:
  http::status_code status_code_ = http::status::ok;
  http_fields fields_;
  optional<any_async_readable_stream> body_;
};

}

FITORIA_NAMESPACE_END

#endif
