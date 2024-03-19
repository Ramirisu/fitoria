//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_RECORD_HPP
#define FITORIA_LOG_RECORD_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/source_location.hpp>

#include <fitoria/log/detail/path.hpp>

#include <fitoria/log/level.hpp>

#include <chrono>
#include <memory>
#include <string>

FITORIA_NAMESPACE_BEGIN

namespace log {

struct record {
  using clock_t = std::chrono::system_clock;

  clock_t::time_point time;
  source_location loc;
  level lv;
  std::string msg;

  record(clock_t::time_point time,
         source_location loc,
         level lv,
         std::string msg)
      : time(time)
      , loc(loc)
      , lv(lv)
      , msg(std::move(msg))
  {
  }
};

using record_ptr = std::shared_ptr<record>;

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
        return fmt::color::green;
      case level::warning:
        return fmt::color::yellow;
      case level::error:
        return fmt::color::red;
      case level::fatal:
        return fmt::color::dark_red;
      default:
        break;
      }

      return fmt::color::white;
    };

    return fmt::format("{}", fmt::styled(to_string(lv), fmt::fg(get_color())));
  }
#endif

  return to_string(lv);
}

inline auto format(record_ptr rec, bool colorful) -> std::string
{
#if defined(FITORIA_TARGET_WINDOWS)
  auto newline = std::string_view("\r\n");
#else
  auto newline = std::string_view("\n");
#endif

#if defined(FITORIA_HAS_STD_SOURCE_LOCATION)
  return fmt::format("[{:%FT%TZ} {} {}] {} [{}:{}:{}]{}",
                     std::chrono::floor<std::chrono::seconds>(rec->time),
                     format(rec->lv, colorful),
                     rec->loc.function_name(),
                     rec->msg,
                     detail::get_file_name(rec->loc.file_name()),
                     rec->loc.line(),
                     rec->loc.column(),
                     newline);
#else
  return fmt::format("[{:%FT%TZ} {}] {}{}",
                     std::chrono::floor<std::chrono::seconds>(rec->time),
                     format(rec->lv, colorful),
                     rec->msg,
                     newline);
#endif
}
}

FITORIA_NAMESPACE_END

#endif
