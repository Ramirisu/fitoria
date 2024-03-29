//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_JSON_OF_HPP
#define FITORIA_WEB_JSON_OF_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>

#include <fitoria/web/detail/as_json.hpp>
#include <fitoria/web/error.hpp>
#include <fitoria/web/from_http_request.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename T>
class json_of : public T {
public:
  static_assert(std::same_as<T, std::remove_cvref_t<T>>,
                "T must not be cvref qualified");

  json_of(T value)
      : T(std::move(value))
  {
  }

  friend auto tag_invoke(from_http_request_t<json_of<T>>, http_request& req)
      -> net::awaitable<expected<json_of<T>, std::error_code>>
  {
    if (req.fields().get(http::field::content_type)
        != http::fields::content_type::json()) {
      co_return unexpected { make_error_code(error::unexpected_content_type) };
    }

    auto str = co_await async_read_all_as<std::string>(req.body());
    if (str) {
      if (*str) {
        co_return detail::as_json<T>(**str);
      }

      co_return unexpected { (*str).error() };
    }

    co_return detail::as_json<T>("");
  }
};

}

FITORIA_NAMESPACE_END

#endif
