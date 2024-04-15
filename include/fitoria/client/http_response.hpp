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

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_read_into_stream_file.hpp>
#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/detail/as_json.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/http_fields.hpp>

#include <fitoria/client/error.hpp>

FITORIA_NAMESPACE_BEGIN

namespace client {

class http_response {
public:
  http_response(web::http::status_code status_code,
                web::http_fields fields,
                optional<web::any_async_readable_stream> body)
      : status_code_(status_code)
      , fields_(std::move(fields))
      , body_(std::move(body))
  {
  }

  http_response(const http_response&) = delete;

  http_response& operator=(const http_response&) = delete;

  http_response(http_response&&) = default;

  http_response& operator=(http_response&&) = default;

  auto status_code() const noexcept -> web::http::status_code
  {
    return status_code_;
  }

  auto fields() noexcept -> web::http_fields&
  {
    return fields_;
  }

  auto fields() const noexcept -> const web::http_fields&
  {
    return fields_;
  }

  auto body() noexcept -> optional<web::any_async_readable_stream&>
  {
    return optional<web::any_async_readable_stream&>(body_);
  }

  auto body() const noexcept -> optional<const web::any_async_readable_stream&>
  {
    return optional<const web::any_async_readable_stream&>(body_);
  }

  auto as_string() -> awaitable<expected<std::string, std::error_code>>
  {
    if (body_) {
      co_return co_await web::async_read_until_eof<std::string>(*body_);
    }

    co_return unexpected { make_error_code(net::error::eof) };
  }

  template <typename Byte>
  auto as_vector() -> awaitable<expected<std::vector<Byte>, std::error_code>>
  {
    if (body_) {
      co_return co_await web::async_read_until_eof<std::vector<Byte>>(*body_);
    }

    co_return unexpected { make_error_code(net::error::eof) };
  }

#if defined(BOOST_ASIO_HAS_FILE)
  auto as_file(const std::string& path)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    if (body_) {
      auto file = net::stream_file(co_await net::this_coro::executor);

      boost::system::error_code ec;
      file.open(path, net::file_base::create | net::file_base::write_only, ec);
      if (ec) {
        co_return unexpected { ec };
      }

      co_return co_await async_read_into_stream_file(*body_, file);
    }

    co_return unexpected { make_error_code(net::error::eof) };
  }
#endif

  template <typename T = boost::json::value>
  auto as_json() -> awaitable<expected<T, std::error_code>>
  {
    if (body_) {
      if (fields().get(web::http::field::content_type)
          != web::http::fields::content_type::json()) {
        co_return unexpected { make_error_code(
            error::unexpected_content_type_json) };
      }

      if (auto str = co_await web::async_read_until_eof<std::string>(*body_);
          str) {
        co_return web::detail::as_json<T>(*str);
      } else {
        co_return unexpected { str.error() };
      }
    }

    co_return unexpected { make_error_code(net::error::eof) };
  }

private:
  web::http::status_code status_code_ = web::http::status::ok;
  web::http_fields fields_;
  optional<web::any_async_readable_stream> body_;
};

}

FITORIA_NAMESPACE_END

#endif
