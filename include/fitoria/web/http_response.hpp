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

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_read_into_stream_file.hpp>
#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/async_readable_eof_stream.hpp>
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

  auto as_string() -> net::awaitable<expected<std::string, std::error_code>>
  {
    return async_read_until_eof<std::string>(body_);
  }

  template <typename Byte>
  auto as_vector()
      -> net::awaitable<expected<std::vector<Byte>, std::error_code>>
  {
    return async_read_until_eof<std::vector<Byte>>(body_);
  }

#if defined(BOOST_ASIO_HAS_FILE)
  auto as_file(const std::string& path)
      -> net::awaitable<expected<std::size_t, std::error_code>>
  {
    auto file = net::stream_file(co_await net::this_coro::executor);

    boost::system::error_code ec;
    file.open(path, net::file_base::create | net::file_base::write_only, ec);
    if (ec) {
      co_return unexpected { ec };
    }

    co_return co_await async_read_into_stream_file(body_, file);
  }
#endif

  template <typename T = boost::json::value>
  auto as_json() -> net::awaitable<expected<T, std::error_code>>
  {
    if (fields().get(http::field::content_type)
        != http::fields::content_type::json()) {
      co_return unexpected { make_error_code(error::unexpected_content_type) };
    }

    if (auto str = co_await async_read_until_eof<std::string>(body()); str) {
      co_return detail::as_json<T>(*str);
    } else {
      co_return unexpected { str.error() };
    }
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
  any_async_readable_stream body_ { async_readable_eof_stream() };
};

}

FITORIA_NAMESPACE_END

#endif
