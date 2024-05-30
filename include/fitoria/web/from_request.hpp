//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_FROM_REQUEST_HPP
#define FITORIA_WEB_FROM_REQUEST_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/tag_invoke.hpp>
#include <fitoria/core/type_traits.hpp>

#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/request.hpp>
#include <fitoria/web/response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

namespace from_request_ns {
  template <typename R>
  struct from_request_t {
    constexpr auto operator()(request& req) const
        noexcept(is_nothrow_tag_invocable_v<from_request_t<R>, request&>)
            -> awaitable<expected<R, response>>
      requires is_tag_invocable_v<from_request_t<R>, request&>
    {
      static_assert(
          std::same_as<tag_invoke_result_t<from_request_t<R>, request&>,
                       awaitable<expected<R, response>>>);
      return tag_invoke(*this, req);
    }

    friend auto tag_invoke(from_request_t<R>,
                           request& req) -> awaitable<expected<R, response>>
      requires(std::same_as<R, request&> || std::same_as<R, const request&>)
    {
      co_return req;
    }

    friend auto tag_invoke(from_request_t<R>,
                           request& req) -> awaitable<expected<R, response>>
      requires(std::same_as<R, const connect_info&>)
    {
      co_return req.connection();
    }

    friend auto tag_invoke(from_request_t<R>,
                           request& req) -> awaitable<expected<R, response>>
      requires(std::same_as<R, const path_info&>)
    {
      co_return req.path();
    }

    friend auto tag_invoke(from_request_t<R>,
                           request& req) -> awaitable<expected<R, response>>
      requires(std::same_as<R, const query_map&>)
    {
      co_return req.query();
    }

    friend auto tag_invoke(from_request_t<R>,
                           request& req) -> awaitable<expected<R, response>>
      requires(decay_to<R, http::version>)
    {
      co_return req.version();
    }

    friend auto tag_invoke(from_request_t<R>,
                           request& req) -> awaitable<expected<R, response>>
      requires(std::same_as<R, const http::header&>)
    {
      co_return req.header();
    }

    friend auto tag_invoke(from_request_t<R>,
                           request& req) -> awaitable<expected<R, response>>
      requires(std::same_as<R, std::string>)
    {
      if (auto result = co_await async_read_until_eof<R>(req.body()); result) {
        co_return std::move(*result);
      } else {
        co_return unexpected { response::bad_request()
                                   .set_header(http::field::content_type,
                                               mime::text_plain())
                                   .set_body("unable to read request body.") };
      }
    }

    friend auto tag_invoke(from_request_t<R>,
                           request& req) -> awaitable<expected<R, response>>
      requires(is_specialization_of_v<R, std::vector>)
    {
      static_assert(not_cvref<R>, "R must not be cvref qualified");

      if (auto result = co_await async_read_until_eof<R>(req.body()); result) {
        co_return std::move(*result);
      } else {
        co_return unexpected { response::bad_request()
                                   .set_header(http::field::content_type,
                                               mime::text_plain())
                                   .set_body("unable to read request body.") };
      }
    }
  };

}

using from_request_ns::from_request_t;
template <typename R>
inline constexpr from_request_t<R> from_request {};

}

FITORIA_NAMESPACE_END

#endif
