//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_FROM_HTTP_REQUEST_HPP
#define FITORIA_WEB_FROM_HTTP_REQUEST_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/tag_invoke.hpp>
#include <fitoria/core/type_traits.hpp>

#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

namespace from_http_request_ns {
  template <typename R>
  struct from_http_request_t {
    constexpr auto operator()(http_request& req) const noexcept(
        is_nothrow_tag_invocable_v<from_http_request_t<R>, http_request&>)
      requires is_tag_invocable_v<from_http_request_t<R>, http_request&>
    {
      static_assert(std::same_as<
                    tag_invoke_result_t<from_http_request_t<R>, http_request&>,
                    awaitable<expected<R, std::error_code>>>);
      return tag_invoke(*this, req);
    }

    friend auto tag_invoke(from_http_request_t<R>, http_request& req)
        -> awaitable<expected<R, std::error_code>>
      requires(std::same_as<R, http_request&>
               || std::same_as<R, const http_request&>)
    {
      co_return req;
    }

    friend auto tag_invoke(from_http_request_t<R>, http_request& req)
        -> awaitable<expected<R, std::error_code>>
      requires(std::same_as<R, const connection_info&>)
    {
      co_return req.connection();
    }

    friend auto tag_invoke(from_http_request_t<R>, http_request& req)
        -> awaitable<expected<R, std::error_code>>
      requires(std::same_as<R, const path_info&>)
    {
      co_return req.path();
    }

    friend auto tag_invoke(from_http_request_t<R>, http_request& req)
        -> awaitable<expected<R, std::error_code>>
      requires(std::same_as<R, const query_map&>)
    {
      co_return req.query();
    }

    friend auto tag_invoke(from_http_request_t<R>, http_request& req)
        -> awaitable<expected<R, std::error_code>>
      requires(std::same_as<R, const http_fields&>)
    {
      co_return req.fields();
    }

    friend auto tag_invoke(from_http_request_t<R>, http_request& req)
        -> awaitable<expected<R, std::error_code>>
      requires(std::same_as<R, std::string>)
    {
      if (req.body()) {
        co_return co_await async_read_until_eof<R>(*req.body());
      }

      co_return unexpected { make_error_code(net::error::eof) };
    }

    friend auto tag_invoke(from_http_request_t<R>, http_request& req)
        -> awaitable<expected<R, std::error_code>>
      requires(is_specialization_of_v<R, std::vector>)
    {
      static_assert(not_cvref<R>, "R must not be cvref qualified");

      if (req.body()) {
        co_return co_await async_read_until_eof<R>(*req.body());
      }

      co_return unexpected { make_error_code(net::error::eof) };
    }
  };

}

using from_http_request_ns::from_http_request_t;
template <typename R>
inline constexpr from_http_request_t<R> from_http_request {};

}

FITORIA_NAMESPACE_END

#endif
