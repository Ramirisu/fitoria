//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <boost/core/detail/string_view.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <variant>

FITORIA_NAMESPACE_BEGIN

namespace detail {

struct string_hash {
  using is_transparent = void;

  size_t operator()(const char* s) const
  {
    return std::hash<std::string_view> {}(s);
  }

  size_t operator()(std::string_view sv) const
  {
    return std::hash<std::string_view> {}(sv);
  }

  size_t operator()(const std::string& s) const
  {
    return std::hash<std::string> {}(s);
  }

  size_t operator()(boost::core::string_view sv) const
  {
    return std::hash<std::string_view> {}(
        std::string_view(sv.data(), sv.size()));
  }
};

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
struct repeated_input_variant_function;

template <typename R, typename T, std::size_t N, std::size_t... Ints>
struct repeated_input_variant_function<R, T, N, std::index_sequence<Ints...>> {
private:
  template <std::size_t N2>
  using rebind = typename repeated_input_function<R, T, N2>::type;

public:
  using type = std::variant<rebind<Ints>...>;
};

template <typename R, typename T, std::size_t N>
using repeated_input_variant_function_t
    = repeated_input_variant_function<R, T, N>::type;

}

FITORIA_NAMESPACE_END
