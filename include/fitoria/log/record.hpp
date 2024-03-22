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

#include <fitoria/core/source_location.hpp>

#include <fitoria/log/level.hpp>

#include <chrono>
#include <memory>
#include <string>

FITORIA_NAMESPACE_BEGIN

namespace log {

struct record {
  using clock_t = std::chrono::system_clock;

  clock_t::time_point time;
  std::uint32_t line;
  std::uint32_t column;
  std::string_view file_name;
  std::string_view function_name;
  level lv;
  std::string msg;

  record(clock_t::time_point time,
         source_location loc,
         level lv,
         std::string msg)
      : time(time)
      , line(loc.line())
      , column(loc.column())
      , file_name(loc.file_name())
      , function_name(loc.function_name())
      , lv(lv)
      , msg(std::move(msg))
  {
  }
};

using record_ptr = std::shared_ptr<record>;

}

FITORIA_NAMESPACE_END

#endif
