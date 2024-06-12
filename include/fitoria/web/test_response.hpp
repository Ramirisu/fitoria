//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_TEST_RESPONSE_HPP
#define FITORIA_WEB_TEST_RESPONSE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/http.hpp>
#include <fitoria/mime.hpp>

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_read_into_stream_file.hpp>
#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/detail/as_json.hpp>
#include <fitoria/web/error.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

/// @verbatim embed:rst:leading-slashes
///
/// A type representing server's outgoing response for testing purpose.
///
/// @endverbatim
class test_response {
public:
  test_response(http::status_code status_code,
                http::version version,
                http::header_map headers,
                any_async_readable_stream body)
      : status_code_(status_code)
      , version_(version)
      , headers_(std::move(headers))
      , body_(std::move(body))
  {
  }

  test_response(const test_response&) = delete;

  test_response& operator=(const test_response&) = delete;

  test_response(test_response&&) = default;

  test_response& operator=(test_response&&) = default;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get HTTP status code.
  ///
  /// @endverbatim
  auto status_code() const noexcept -> const http::status_code&
  {
    return status_code_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get HTTP version.
  ///
  /// @endverbatim
  auto version() const noexcept -> const http::version&
  {
    return version_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get HTTP headers.
  ///
  /// @endverbatim
  auto headers() const noexcept -> const http::header_map&
  {
    return headers_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get body.
  ///
  /// @endverbatim
  auto body() noexcept -> any_async_readable_stream&
  {
    return body_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get body.
  ///
  /// @endverbatim
  auto body() const noexcept -> const any_async_readable_stream&
  {
    return body_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Read complete body as ``std::string``.
  ///
  /// @endverbatim
  auto as_string() -> awaitable<expected<std::string, std::error_code>>
  {
    return web::async_read_until_eof<std::string>(body_);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Read complete body as ``std::vector<Byte>``.
  ///
  /// @endverbatim
  template <typename Byte>
  auto as_vector() -> awaitable<expected<std::vector<Byte>, std::error_code>>
  {
    return web::async_read_until_eof<std::vector<Byte>>(body_);
  }

#if defined(BOOST_ASIO_HAS_FILE)

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Read complete body into file.
  ///
  /// @endverbatim
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

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Read complete body into type ``T``.
  ///
  /// @endverbatim
  template <typename T = boost::json::value>
  auto as_json() -> awaitable<expected<T, std::error_code>>
  {
    if (auto mime = headers()
                        .get(http::field::content_type)
                        .and_then(mime::mime_view::parse);
        !mime || mime->essence() != mime::application_json()) {
      co_return unexpected { make_error_code(error::unexpected_content_type) };
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
  http::version version_;
  http::header_map headers_;
  web::any_async_readable_stream body_;
};

}

FITORIA_NAMESPACE_END

#endif
