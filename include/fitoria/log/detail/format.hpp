//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_DETAIL_FORMAT_HPP
#define FITORIA_LOG_DETAIL_FORMAT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>

#include <fitoria/log/detail/path.hpp>

#include <fitoria/log/record.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log::detail {

inline auto format(level lv, [[maybe_unused]] bool colorful) -> std::string
{
#if defined(FITORIA_HAS_FMT)
  if (colorful) {
    auto get_color = [lv]() -> fmt::color {
      switch (lv) {
      case level::trace:
        return fmt::color::white;
      case level::debug:
        return fmt::color::cyan;
      case level::info:
        return fmt::color::lime_green;
      case level::warning:
        return fmt::color::yellow;
      case level::error:
        return fmt::color::red;
      case level::fatal:
        return fmt::color::magenta;
      default:
        break;
      }

      return fmt::color::white;
    };

    return fmt::format("{}", fmt::styled(lv, fmt::fg(get_color())));
  }
#endif

  return fmt::format("{}", lv);
}

inline auto newline()
{
#if defined(FITORIA_TARGET_WINDOWS)
  return std::string_view("\r\n");
#else
  return std::string_view("\n");
#endif
}

inline auto format(record_ptr rec, bool colorful) -> std::string
{
  return fmt::format("[{:%FT%TZ} {} {}] {} [{}:{}:{}]{}",
                     std::chrono::floor<std::chrono::seconds>(rec->time),
                     format(rec->lv, colorful),
                     rec->loc.function_name(),
                     rec->msg,
                     get_file_name(rec->loc.file_name()),
                     rec->loc.line(),
                     rec->loc.column(),
                     newline());
}

}

FITORIA_NAMESPACE_END

#endif
