//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_DETAIL_COROUTINE_CONCEPT_HPP
#define FITORIA_CORE_DETAIL_COROUTINE_CONCEPT_HPP

#include <fitoria/core/config.hpp>

#include <utility>

FITORIA_NAMESPACE_BEGIN

namespace detail {

template <typename Awaiter>
concept awaiter = requires(Awaiter&& awaiter) {
  {
    awaiter.await_ready()
  } -> std::convertible_to<bool>;
  awaiter.await_resume();
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

#endif
