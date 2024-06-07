//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ASYNC_READABLE_STREAM_CONCEPT_HPP
#define FITORIA_WEB_ASYNC_READABLE_STREAM_CONCEPT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/bytes.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/optional.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

/// @verbatim embed:rst:leading-slashes
///
/// Defines the concept for an async readable stream.
///
/// @endverbatim
template <typename T>
concept async_readable_stream = requires(T t) {
  typename std::remove_cvref_t<T>::is_async_readable_stream;
  {
    t.async_read_some()
  } -> std::same_as<awaitable<optional<expected<bytes, std::error_code>>>>;
};

}

FITORIA_NAMESPACE_END

#endif
