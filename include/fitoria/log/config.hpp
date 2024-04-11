//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_CONFIG_HPP
#define FITORIA_LOG_CONFIG_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

using executor_type = boost::asio::system_context::executor_type;

template <typename T>
using awaitable = boost::asio::awaitable<T, executor_type>;

constexpr auto use_awaitable
    = boost::asio::as_tuple_t<boost::asio::use_awaitable_t<executor_type>> {};
}

FITORIA_NAMESPACE_END

#endif
