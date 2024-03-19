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
#endif

FITORIA_NAMESPACE_BEGIN

#if defined(FITORIA_HAS_STD_SOURCE_LOCATION)

using std::source_location;

#else

class source_location {
public:
  source_location() = default;

  static constexpr source_location current() noexcept
  {
    return source_location();
  }
};

#endif

FITORIA_NAMESPACE_END

#endif
