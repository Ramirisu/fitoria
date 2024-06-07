//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_TO_RESPONSE_HPP
#define FITORIA_WEB_TO_RESPONSE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/tag_invoke.hpp>

#include <fitoria/web/response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

namespace to_response_ns {
  struct to_response_t {
    template <typename T>
      requires is_tag_invocable_v<to_response_t, T>
    auto operator()(T&& t) const
        noexcept(is_nothrow_tag_invocable_v<to_response_t, T>) -> response
    {
      static_assert(
          std::same_as<tag_invoke_result_t<to_response_t, T>, response>);
      return tag_invoke(*this, std::forward<T>(t));
    }

    template <decay_to<response> Self>
    auto operator()(Self self) const -> response
    {
      return self;
    }

    template <decay_to<std::string> String>
    auto operator()(String str) const -> response
    {
      return response::ok()
          .set_header(http::field::content_type, mime::text_plain())
          .set_body(str);
    }

    template <typename T>
    auto operator()(std::vector<T> vec) const -> response
    {
      return response::ok()
          .set_header(http::field::content_type,
                      mime::application_octet_stream())
          .set_body(std::as_bytes(std::span(vec.begin(), vec.end())));
    }

    template <typename... Ts>
    auto operator()(std::variant<Ts...> var) const -> response
    {
      return std::visit(
          [this]<typename Arg>(Arg&& arg) -> response {
            return (*this)(std::forward<Arg>(arg));
          },
          std::move(var));
    }

    template <typename T, typename E>
    auto operator()(expected<T, E> exp) const -> response
    {
      return exp ? (*this)(std::move(*exp)) : (*this)(std::move(exp.error()));
    }
  };
}

using to_response_ns::to_response_t;
inline constexpr to_response_t to_response {};

}

FITORIA_NAMESPACE_END

#endif
