//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CLIENT_HTTP_RESPONSE_HPP
#define FITORIA_CLIENT_HTTP_RESPONSE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/http.hpp>

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_read_into_stream_file.hpp>
#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/detail/as_json.hpp>
#include <fitoria/web/http_fields.hpp>

#include <fitoria/client/error.hpp>

FITORIA_NAMESPACE_BEGIN

namespace client {

class http_response {
public:
  http_response(http::status_code status_code,
                http::version version,
                web::http_fields fields,
                web::any_async_readable_stream body)
      : status_code_(status_code)
      , version_(version)
      , fields_(std::move(fields))
      , body_(std::move(body))
  {
  }

  http_response(const http_response&) = delete;

  http_response& operator=(const http_response&) = delete;

  http_response(http_response&&) = default;

  http_response& operator=(http_response&&) = default;

  auto status_code() const noexcept -> http::status_code
  {
    return status_code_;
  }

  auto version() const noexcept -> http::version
  {
    return version_;
  }

  auto fields() noexcept -> web::http_fields&
  {
    return fields_;
  }

  auto fields() const noexcept -> const web::http_fields&
  {
    return fields_;
  }

  auto body() noexcept -> web::any_async_readable_stream&
  {
    return body_;
  }

  auto body() const noexcept -> const web::any_async_readable_stream&
  {
    return body_;
  }

  auto as_string() -> awaitable<expected<std::string, std::error_code>>
  {
    return web::async_read_until_eof<std::string>(body_);
  }

  template <typename Byte>
  auto as_vector() -> awaitable<expected<std::vector<Byte>, std::error_code>>
  {
    return web::async_read_until_eof<std::vector<Byte>>(body_);
  }

#if defined(BOOST_ASIO_HAS_FILE)
  auto as_file(const std::string& path)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    auto file = net::stream_file(co_await net::this_coro::executor);

    boost::system::error_code ec; // NOLINTNEXTLINE
    file.open(path, net::file_base::create | net::file_base::write_only, ec);
    if (ec) {
      co_return unexpected { ec };
    }

    co_return co_await async_read_into_stream_file(body_, file);
  }
#endif

  template <typename T = boost::json::value>
  auto as_json() -> awaitable<expected<T, std::error_code>>
  {
    if (fields().get(http::field::content_type)
        != http::fields::content_type::json()) {
      co_return unexpected { make_error_code(
          error::content_type_not_application_json) };
    }

    if (auto str = co_await web::async_read_until_eof<std::string>(body_);
        str) {
      co_return web::detail::as_json<T>(*str);
    } else {
      co_return unexpected { str.error() };
    }
  }

private:
  http::status_code status_code_ = http::status::ok;
  http::version version_ = http::version::v1_1;
  web::http_fields fields_;
  web::any_async_readable_stream body_;
};

}

FITORIA_NAMESPACE_END

#endif
