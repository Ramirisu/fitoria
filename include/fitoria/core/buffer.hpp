//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_BUFFER_HPP
#define FITORIA_CORE_BUFFER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

#include <span>

FITORIA_NAMESPACE_BEGIN

template <typename T, std::size_t N>
auto as_const_buffer(std::span<T, N> buffer) -> net::const_buffer
{
  auto bytes = std::as_bytes(buffer);
  return { bytes.data(), bytes.size() };
}

FITORIA_NAMESPACE_END

#endif
