//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/fwd.hpp>

#include <fitoria/core/handler_concept.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/http_server/http_error.hpp>
#include <fitoria/http_server/http_response.hpp>

#include <functional>
#include <variant>

FITORIA_NAMESPACE_BEGIN

namespace detail {

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

struct handler_trait {
  using result_t = net::awaitable<expected<http_response, http_error>>;

public:
  using handler_t
      = repeated_input_variant_function_t<result_t, http_context&, 5>;
  using handlers_t = std::vector<handler_t>;
  using handler_result_t = result_t;
  static constexpr bool handler_result_awaitable = true;
  struct handler_compare_t;
};

}

FITORIA_NAMESPACE_END
