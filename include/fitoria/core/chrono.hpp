//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_CHRONO_HPP
#define FITORIA_CORE_CHRONO_HPP

#include <fitoria/core/config.hpp>

#include <chrono>

FITORIA_NAMESPACE_BEGIN

namespace chrono {

#if defined(FITORIA_TARGET_WINDOWS)

using utc_clock = std::chrono::utc_clock;

template <typename Duration>
inline auto to_utc(const std::chrono::file_time<Duration>& time)
    -> decltype(std::chrono::file_clock::to_utc(time))
{
  return std::chrono::file_clock::to_utc(time);
}

template <typename Duration>
inline auto from_utc(const std::chrono::time_point<utc_clock, Duration>& time)
    -> decltype(std::chrono::file_clock::from_utc(time))
{
  return std::chrono::file_clock::from_utc(time);
}

#else

using utc_clock = std::chrono::system_clock;

template <typename Duration>
inline auto to_utc(const std::chrono::file_time<Duration>& time)
    -> decltype(std::chrono::file_clock::to_sys(time))
{
  return std::chrono::file_clock::to_sys(time);
}

template <typename Duration>
inline auto from_utc(const std::chrono::time_point<utc_clock, Duration>& time)
    -> decltype(std::chrono::file_clock::from_sys(time))
{
  return std::chrono::file_clock::from_sys(time);
}

#endif

}

FITORIA_NAMESPACE_END

#endif
