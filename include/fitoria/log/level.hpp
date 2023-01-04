//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

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

}

FITORIA_NAMESPACE_END
