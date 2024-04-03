//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_TYPE_TRAITS_HPP
#define FITORIA_CORE_TYPE_TRAITS_HPP

#include <fitoria/core/config.hpp>

#include <tuple>

FITORIA_NAMESPACE_BEGIN

template <typename T>
concept not_cvref = std::same_as<T, std::remove_cvref_t<T>>;

template <typename T, typename U>
concept decay_to = std::same_as<std::decay_t<T>, U>;

template <typename T, typename U>
concept not_decay_to = (!decay_to<T, U>);

template <typename T, template <typename...> class U>
struct is_specialization_of : std::false_type { };

template <template <typename...> class U, typename... Args>
struct is_specialization_of<U<Args...>, U> : std::true_type { };

template <typename T, template <typename...> class U>
inline constexpr bool is_specialization_of_v
    = is_specialization_of<T, U>::value;

template <typename F>
struct function_traits_helper;

template <typename R, typename... Args>
struct function_traits_helper<R(Args...)> {
  using result_type = R;

  static constexpr std::size_t arity = sizeof...(Args);

  using args_type = std::tuple<Args...>;

  template <std::size_t Index>
  struct arg {
    using type = std::tuple_element_t<Index, args_type>;
  };
};

template <typename F>
struct function_traits : function_traits<decltype(&F::operator())> { };

template <typename R, typename... Args>
struct function_traits<R(Args...)> : function_traits_helper<R(Args...)> { };

template <typename R, typename... Args>
struct function_traits<R (*)(Args...)> : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...)> : function_traits_helper<R(Args...)> {
};

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const>
    : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...)&>
    : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const&>
    : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) &&>
    : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const&&>
    : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) noexcept>
    : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const noexcept>
    : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) & noexcept>
    : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const & noexcept>
    : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) && noexcept>
    : function_traits_helper<R(Args...)> { };

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const && noexcept>
    : function_traits_helper<R(Args...)> { };

FITORIA_NAMESPACE_END

#endif
