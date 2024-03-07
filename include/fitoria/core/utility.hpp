//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_UTILITY_HPP
#define FITORIA_CORE_UTILITY_HPP

#include <fitoria/core/config.hpp>

#include <tuple>
#include <type_traits>

FITORIA_NAMESPACE_BEGIN

template <typename E>
auto to_underlying(E e) noexcept
{
  return static_cast<std::underlying_type_t<E>>(e);
}

template <typename... Ts, std::size_t... Is>
auto reverse_tuple_impl(std::tuple<Ts...> t, std::index_sequence<Is...>)
{
  return std::tuple { std::get<sizeof...(Ts) - Is - 1>(std::move(t))... };
}

template <typename... Ts>
auto reverse_tuple(std::tuple<Ts...> t)
{
  return reverse_tuple_impl(std::move(t),
                            std::make_index_sequence<sizeof...(Ts)>());
}

FITORIA_NAMESPACE_END

#endif
