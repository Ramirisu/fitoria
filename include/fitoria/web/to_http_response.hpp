//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_TO_HTTP_RESPONSE_HPP
#define FITORIA_WEB_TO_HTTP_RESPONSE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/tag_invoke.hpp>

#include <fitoria/web/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

namespace to_http_response_ns {
  struct to_http_response_t {
    template <typename T>
      requires is_tag_invocable_v<to_http_response_t, T>
    auto operator()(T&& t) const
        noexcept(is_nothrow_tag_invocable_v<to_http_response_t, T>)
            -> http_response
    {
      static_assert(std::same_as<tag_invoke_result_t<to_http_response_t, T>,
                                 http_response>);
      return tag_invoke(*this, std::forward<T>(t));
    }

    template <typename T>
      requires decay_to<T, http_response>
    auto operator()(T&& t) const -> http_response
    {
      return std::forward<T>(t);
    }

    template <typename T>
      requires decay_to<T, std::string>
    auto operator()(T&& t) const -> http_response
    {
      return http_response(http::status::ok)
          .set_field(http::field::content_type,
                     http::fields::content_type::plaintext())
          .set_body(std::forward<T>(t));
    }

    template <typename T>
      requires is_specialization_of_v<T, std::vector>
    auto operator()(T&& t) const -> http_response
    {
      return http_response(http::status::ok)
          .set_field(http::field::content_type,
                     http::fields::content_type::octet_stream())
          .set_body(std::as_bytes(std::span(t.begin(), t.end())));
    }

    template <typename T>
      requires is_specialization_of_v<T, std::variant>
    auto operator()(T&& t) const -> http_response
    {
      return std::visit(
          [this]<typename Arg>(Arg&& arg) -> http_response {
            return (*this)(std::forward<Arg>(arg));
          },
          std::forward<T>(t));
    }
  };
}

using to_http_response_ns::to_http_response_t;
inline constexpr to_http_response_t to_http_response {};

}

FITORIA_NAMESPACE_END

#endif
