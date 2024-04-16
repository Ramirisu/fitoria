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

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/web/http/http.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename T>
concept async_readable_stream = requires(T t, net::mutable_buffer buffer) {
  typename std::remove_cvref_t<T>::is_async_readable_stream;
  {
    t.async_read_some(buffer)
  } -> std::same_as<awaitable<expected<std::size_t, std::error_code>>>;
};

}

FITORIA_NAMESPACE_END

#endif
