//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_STATE_HPP
#define FITORIA_WEB_STATE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/error.hpp>
#include <fitoria/core/expected.hpp>

#include <fitoria/web/from_http_request.hpp>
#include <fitoria/web/state_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename T>
  requires std::copy_constructible<T>
class state : public T {
public:
  static_assert(std::same_as<T, std::remove_cvref_t<T>>,
                "T must not be cvref qualified");

  state(T value)
      : T(std::move(value))
  {
  }

  friend auto tag_invoke(from_http_request_t<state<T>>, http_request& req)
      -> net::awaitable<expected<state<T>, error_code>>
  {
    if (auto result = req.state<T>(); result) {
      co_return state<T>(*result);
    }

    co_return unexpected { make_error_code(error::shared_state_not_exists) };
  }
};

}

FITORIA_NAMESPACE_END

#endif
