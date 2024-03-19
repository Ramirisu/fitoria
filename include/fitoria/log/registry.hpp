//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_REGISTRY_HPP
#define FITORIA_LOG_REGISTRY_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>

#include <fitoria/log/async_logger.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

class registry {
  std::shared_ptr<async_logger> default_;

public:
  static auto global() -> registry&
  {
    static registry inst;
    return inst;
  }

  void set_default_logger(std::shared_ptr<async_logger> logger)
  {
    default_ = std::move(logger);
  }

  auto default_logger() -> optional<async_logger&>
  {
    if (default_) {
      return optional<async_logger&>(*default_);
    }

    return nullopt;
  }
};

}

FITORIA_NAMESPACE_END

#endif
