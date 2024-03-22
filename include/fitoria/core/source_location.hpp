//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_SOURCE_LOCATION_HPP
#define FITORIA_CORE_SOURCE_LOCATION_HPP

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_STD_SOURCE_LOCATION)
#include <source_location>
#else
#include <cstdint>
#endif

FITORIA_NAMESPACE_BEGIN

#if defined(FITORIA_HAS_STD_SOURCE_LOCATION)

using std::source_location;

#else

struct source_location {
  static consteval source_location
  current(const std::uint_least32_t line = __builtin_LINE(),
          const std::uint_least32_t column = __builtin_COLUMN(),
          const char* const file = __builtin_FILE(),
          const char* const function = __builtin_FUNCTION()) noexcept
  {
    source_location loc {};
    loc.line_ = line;
    loc.column_ = column;
    loc.file_ = file;
    loc.function_ = function;
    return loc;
  }

  constexpr source_location() noexcept = default;

  constexpr std::uint_least32_t line() const noexcept
  {
    return line_;
  }
  constexpr std::uint_least32_t column() const noexcept
  {
    return column_;
  }
  constexpr const char* file_name() const noexcept
  {
    return file_;
  }
  constexpr const char* function_name() const noexcept
  {
    return function_;
  }

private:
  std::uint_least32_t line_ {};
  std::uint_least32_t column_ {};
  const char* file_ = "";
  const char* function_ = "";
};

#endif

FITORIA_NAMESPACE_END

#endif
