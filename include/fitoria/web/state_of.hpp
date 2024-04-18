//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_STATE_OF_HPP
#define FITORIA_WEB_STATE_OF_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>

#include <fitoria/web/from_request.hpp>
#include <fitoria/web/state_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename T>
class state_of : public T {
public:
  static_assert(std::copy_constructible<T>);
  static_assert(not_cvref<T>, "T must not be cvref qualified");

  state_of(T inner)
      : T(std::move(inner))
  {
  }

  friend auto tag_invoke(from_request_t<state_of<T>>, request& req)
      -> awaitable<expected<state_of<T>, std::error_code>>
  {
    if (auto result = req.state<T>(); result) {
      co_return state_of<T>(*result);
    }

    co_return unexpected { make_error_code(error::state_not_found) };
  }
};

}

FITORIA_NAMESPACE_END

#endif
