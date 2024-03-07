//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_UTILITY_HPP
#define FITORIA_CORE_UTILITY_HPP

#include <fitoria/core/config.hpp>

#include <type_traits>

FITORIA_NAMESPACE_BEGIN

template <typename E>
auto to_underlying(E e) noexcept
{
  return static_cast<std::underlying_type_t<E>>(e);
}

FITORIA_NAMESPACE_END

#endif
