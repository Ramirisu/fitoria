//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <concepts>
#include <utility>

FITORIA_NAMESPACE_BEGIN

template <typename Awaitable>
concept member_co_awaitable
    = requires { std::declval<Awaitable>().operator co_await(); };

template <typename Awaitable>
concept free_co_awaitable
    = requires { operator co_await(std::declval<Awaitable>()); };

template <typename Awaiter>
concept awaiter = requires(Awaiter awaiter) {
                    // clang-format off
                    { awaiter.await_ready() } -> std::same_as<bool>;
                    awaiter.await_resume();
                    // clang-format on
                  };

template <typename Awaitable>
concept awaitable = member_co_awaitable<Awaitable>
    || free_co_awaitable<Awaitable> || awaiter<Awaitable>;

FITORIA_NAMESPACE_END
