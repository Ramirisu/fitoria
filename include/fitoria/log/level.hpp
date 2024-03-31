//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_LEVEL_HPP
#define FITORIA_LOG_LEVEL_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>

#include <algorithm>
#include <cstdint>
#include <string>

FITORIA_NAMESPACE_BEGIN

namespace log {

enum class level : std::uint8_t {
  trace = 0,
  debug,
  info,
  warning,
  error,
  fatal,
  off,

  // internal use only
  count,
};

inline std::string to_string(level lv)
{
  switch (lv) {
  case level::trace:
    return "TRACE";
  case level::debug:
    return "DEBUG";
  case level::info:
    return "INFO";
  case level::warning:
    return "WARNING";
  case level::error:
    return "ERROR";
  case level::fatal:
    return "FATAL";
  default:
    break;
  }

  return "UNKNOWN";
}

inline level to_level(std::string_view sv)
{
  std::string lower(sv.size(), '\0');
  std::transform(sv.begin(), sv.end(), lower.begin(), [](char c) {
    return static_cast<char>(std::tolower(c));
  });
  if (lower == "trace") {
    return level::trace;
  }
  if (lower == "debug") {
    return level::debug;
  }
  if (lower == "info") {
    return level::info;
  }
  if (lower == "warning") {
    return level::warning;
  }
  if (lower == "error") {
    return level::error;
  }
  if (lower == "fatal") {
    return level::fatal;
  }

  return level::off;
}

}

FITORIA_NAMESPACE_END

template <typename CharT>
struct FITORIA_NAMESPACE::fmt::formatter<FITORIA_NAMESPACE::log::level, CharT> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    auto it = ctx.begin();
    if (it == ctx.end()) {
      return it;
    }
    if (*it != '}') {
      FITORIA_THROW_OR(FITORIA_NAMESPACE::fmt::format_error(
                           "Invalid format args for fitoria::log::level"),
                       std::terminate());
    }

    return it;
  }

  template <typename FormatContext>
  auto format(FITORIA_NAMESPACE::log::level lv, FormatContext& ctx) const
  {
    auto str = to_string(lv);
    return std::copy(str.begin(), str.end(), ctx.out());
  }
};

#endif
