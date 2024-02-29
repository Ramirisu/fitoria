//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_SOURCE_LOCATION_HPP
#define FITORIA_CORE_SOURCE_LOCATION_HPP

#include <fitoria/core/config.hpp>

#if defined(__cpp_lib_source_location)
#include <source_location>
#endif

FITORIA_NAMESPACE_BEGIN

#if defined(__cpp_lib_source_location)

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
