//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_FILTER_HPP
#define FITORIA_LOG_FILTER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/utility.hpp>

#include <fitoria/log/level.hpp>

#include <cstdlib>

FITORIA_NAMESPACE_BEGIN

namespace log {

class filter {
  using flags_t = std::uint32_t;

  flags_t flags_ {};

  static flags_t to_bit(level lv) noexcept
  {
    return (1 << static_cast<flags_t>(lv)) >> 1;
  }

  filter() = default;

public:
  filter(std::initializer_list<level> allowed)
  {
    for (auto& lv : allowed) {
      flags_ |= to_bit(lv);
    }
  }

  bool is_allowed(level lv) const noexcept
  {
    return (to_bit(lv) & flags_) > 0;
  }

  static auto all() -> filter
  {
    return at_least(level::trace);
  }

  static auto at_least(level lv) -> filter
  {
    if (lv == level::off) {
      return filter();
    }

    filter flt;
    for (auto l = to_underlying(lv); l <= to_underlying(level::fatal); ++l) {
      flt.flags_ |= to_bit(static_cast<level>(l));
    }
    return flt;
  }

  static auto from_env() -> filter
  {
    if (auto lv_str = std::getenv("CPP_LOG"); lv_str != nullptr) {
      return at_least(to_level(lv_str));
    }

    return filter();
  }
};

}

FITORIA_NAMESPACE_END

#endif
