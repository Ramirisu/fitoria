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
auto from_string_impl(std::string_view s) -> expected<R, std::error_code>
{
  auto value = R {};

  if (auto [_, e] = std::from_chars(s.data(), s.data() + s.size(), value);
      e != std::errc()) {
    return unexpected { make_error_code(e) };
  }

  return value;
}

#if defined(FITORIA_TARGET_MACOS)

template <>
inline auto
from_string_impl<float>(std::string_view s) -> expected<float, std::error_code>
{
  char* end {};
  float value = std::strtof(s.data(), &end);

  if (value == HUGE_VAL) {
    return unexpected { make_error_code(std::errc::result_out_of_range) };
  }
  if (value == 0 && end == s.data()) {
    return unexpected { make_error_code(std::errc::invalid_argument) };
  }

  return value;
}

template <>
inline auto from_string_impl<double>(std::string_view s)
    -> expected<double, std::error_code>
{
  char* end {};
  double value = std::strtod(s.data(), &end);

  if (value == HUGE_VALF) {
    return unexpected { make_error_code(std::errc::result_out_of_range) };
  }
  if (value == 0 && end == s.data()) {
    return unexpected { make_error_code(std::errc::invalid_argument) };
  }

  return value;
}

template <>
inline auto from_string_impl<long double>(std::string_view s)
    -> expected<long double, std::error_code>
{
  char* end {};
  long double value = std::strtold(s.data(), &end);

  if (value == HUGE_VALL) {
    return unexpected { make_error_code(std::errc::result_out_of_range) };
  }
  if (value == 0 && end == s.data()) {
    return unexpected { make_error_code(std::errc::invalid_argument) };
  }

  return value;
}

#endif

}

FITORIA_NAMESPACE_END

#endif
