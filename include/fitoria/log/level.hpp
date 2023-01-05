//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <algorithm>
#include <cctype>
#include <string_view>

FITORIA_NAMESPACE_BEGIN

namespace log {

enum class level {
  debug,
  info,
  warning,
  error,
  fatal,
  off,
};

std::string_view to_string(level lv)
{
  switch (lv) {
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

level to_level(std::string_view sv)
{
  std::string lower(sv.size(), '\0');
  std::transform(sv.begin(), sv.end(), lower.begin(),
                 [](char c) { return static_cast<char>(std::tolower(c)); });
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
