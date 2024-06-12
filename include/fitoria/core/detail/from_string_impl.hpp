//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_DETAIL_FROM_STRING_HPP
#define FITORIA_CORE_DETAIL_FROM_STRING_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>

#include <charconv>
#include <string_view>

#if defined(FITORIA_TARGET_MACOS)
#include <cstdlib>
#endif

FITORIA_NAMESPACE_BEGIN

namespace detail {

template <typename R>
auto from_string_impl(std::string_view s) noexcept
    -> expected<R, std::error_code>
{
  auto value = R {};

  const auto first = s.data();
  const auto last = s.data() + s.size();
  auto [ptr, e] = std::from_chars(first, last, value);
  if (e != std::errc()) {
    return unexpected { make_error_code(e) };
  }
  if (ptr != last) {
    return unexpected { make_error_code(std::errc::invalid_argument) };
  }

  return value;
}

#if defined(FITORIA_TARGET_MACOS)

inline auto is_floating_point_like(std::string_view s) noexcept -> bool
{
  if (s.starts_with('-')) {
    s = s.substr(1);
  }

  int dots = 0;
  for (auto c : s) {
    if (std::isdigit(c)) {
      continue;
    }
    if (c == '.') {
      ++dots;
      continue;
    }

    return false;
  }

  return (dots == 0 && !s.empty()) || (dots == 1 && s.size() > 1);
}

template <>
inline auto from_string_impl<float>(std::string_view s) noexcept
    -> expected<float, std::error_code>
{
  if (!is_floating_point_like(s)) {
    return unexpected { make_error_code(std::errc::invalid_argument) };
  }

  char* end {};
  float value = std::strtof(s.data(), &end);

  if (errno == ERANGE) {
    return unexpected { make_error_code(std::errc::result_out_of_range) };
  }

  return value;
}

template <>
inline auto from_string_impl<double>(std::string_view s) noexcept
    -> expected<double, std::error_code>
{
  if (!is_floating_point_like(s)) {
    return unexpected { make_error_code(std::errc::invalid_argument) };
  }

  char* end {};
  double value = std::strtod(s.data(), &end);

  if (errno == ERANGE) {
    return unexpected { make_error_code(std::errc::result_out_of_range) };
  }

  return value;
}

template <>
inline auto from_string_impl<long double>(std::string_view s) noexcept
    -> expected<long double, std::error_code>
{
  if (!is_floating_point_like(s)) {
    return unexpected { make_error_code(std::errc::invalid_argument) };
  }

  char* end {};
  long double value = std::strtold(s.data(), &end);

  if (errno == ERANGE) {
    return unexpected { make_error_code(std::errc::result_out_of_range) };
  }

  return value;
}

#endif

}

FITORIA_NAMESPACE_END

#endif
