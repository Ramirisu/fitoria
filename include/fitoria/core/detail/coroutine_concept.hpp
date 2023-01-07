//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <concepts>
#include <coroutine>

FITORIA_NAMESPACE_BEGIN

namespace detail {

template <typename Awaiter>
concept awaiter = requires(Awaiter&& awaiter) {
                    // clang-format off
  { awaiter.await_ready() } -> std::convertible_to<bool>;
  awaiter.await_resume();
                    // clang-format on
                  };

template <typename Awaitable>
concept member_co_awaitable
    = requires { operator co_await(std::declval<Awaitable>()); };

template <typename Awaitable>
concept free_co_awaitable
    = requires { std::declval<Awaitable>().operator co_await(); };

template <typename Awaitable>
decltype(auto) get_awaiter(Awaitable&& awaitable) noexcept(
    noexcept(std::forward<Awaitable>(awaitable).operator co_await))
  requires member_co_awaitable<Awaitable>
{
  return std::forward<Awaitable>(awaitable).operator co_await;
}

template <typename Awaitable>
decltype(auto) get_awaiter(Awaitable&& awaitable) noexcept(
    noexcept(operator co_await(std::forward<Awaitable>(awaitable))))
  requires free_co_awaitable<Awaitable>
{
  return operator co_await(std::forward<Awaitable>(awaitable));
}

template <typename Awaiter>
decltype(auto) get_awaiter(Awaiter&& awaiter) noexcept(
    noexcept(std::forward<Awaiter>(awaiter)))
  requires(!member_co_awaitable<Awaiter> && !free_co_awaitable<Awaiter>)
{
  return std::forward<Awaiter>(awaiter);
}

}

FITORIA_NAMESPACE_END
