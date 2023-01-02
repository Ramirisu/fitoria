//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <functional>
#include <type_traits>
#include <variant>

FITORIA_NAMESPACE_BEGIN

template <typename R,
          typename T,
          std::size_t N,
          typename = std::make_index_sequence<N>>
struct repeated_input_function;

template <typename R, typename T, std::size_t N, std::size_t... Ints>
struct repeated_input_function<R, T, N, std::index_sequence<Ints...>> {
private:
  template <std::size_t>
  using rebind = T;

public:
  using type = std::function<R(rebind<Ints>...)>;
};

template <typename R,
          typename T,
          std::size_t N,
          typename = std::make_index_sequence<N>>
struct variant_function;

template <typename R, typename T, std::size_t N, std::size_t... Ints>
struct variant_function<R, T, N, std::index_sequence<Ints...>> {
private:
  template <std::size_t N2>
  using rebind = typename repeated_input_function<R, T, N2>::type;

public:
  using type = std::variant<rebind<Ints>...>;
};

template <typename R, typename T, std::size_t N>
using variant_function_t = typename variant_function<R, T, N>::type;

FITORIA_NAMESPACE_END
