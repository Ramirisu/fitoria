//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_ASYNC_WRITER_HPP
#define FITORIA_LOG_ASYNC_WRITER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

#include <fitoria/log/record.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

class async_writer {
public:
  virtual ~async_writer() = default;

  virtual auto async_write(record_ptr rec) -> net::awaitable<void> = 0;
};

}

FITORIA_NAMESPACE_END

#endif
