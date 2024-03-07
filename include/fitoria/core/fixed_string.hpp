//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_FIXED_STRING_HPP
#define FITORIA_CORE_FIXED_STRING_HPP

#include <fitoria/core/config.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

FITORIA_NAMESPACE_BEGIN

template <typename T, std::size_t N>
struct basic_fixed_string {
  std::array<T, N> str {};

  constexpr basic_fixed_string() = default;

  constexpr basic_fixed_string(const T (&s)[N + 1]) noexcept
  {
    std::copy_n(std::begin(s), N, begin());
  }

  constexpr auto data() const noexcept
  {
    return str.data();
  }

  constexpr auto size() const noexcept
  {
    return N;
  }

  constexpr auto begin() noexcept
  {
    return str.begin();
  }

  constexpr auto begin() const noexcept
  {
    return str.begin();
  }

  constexpr auto end() noexcept
  {
    return str.end();
  }

  constexpr auto end() const noexcept
  {
    return str.end();
  }

  constexpr operator std::basic_string_view<T>() const noexcept
  {
    return std::basic_string_view<T>(data(), size());
  }

  template <std::size_t N2>
  constexpr auto
  operator+(const basic_fixed_string<T, N2>& other) const noexcept
  {
    basic_fixed_string<T, N + N2> result;
    auto out_it = std::copy(begin(), end(), result.begin());
    std::copy_n(other.begin(), N2, out_it);
    return result;
  }
};

template <typename T, std::size_t N>
basic_fixed_string(const T (&)[N]) -> basic_fixed_string<T, N - 1>;

FITORIA_NAMESPACE_END

#endif
