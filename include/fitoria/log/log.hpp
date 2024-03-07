//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_LOG_HPP
#define FITORIA_LOG_LOG_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/log/logger.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

template <typename... Args>
struct [[maybe_unused]] log {
  log(level lv,
      Args&&... args,
      const source_location& loc = source_location::current())
  {
    global_logger()->log(loc, lv, std::forward<Args>(args)...);
  }
};

template <typename... Args>
log(level, Args&&...) -> log<Args...>;

template <typename... Args>
struct [[maybe_unused]] debug {
  debug(Args&&... args, const source_location& loc = source_location::current())
  {
    global_logger()->log(loc, level::debug, std::forward<Args>(args)...);
  }
};

template <typename... Args>
debug(Args&&...) -> debug<Args...>;

template <typename... Args>
struct [[maybe_unused]] info {
  info(Args&&... args, const source_location& loc = source_location::current())
  {
    global_logger()->log(loc, level::info, std::forward<Args>(args)...);
  }
};

template <typename... Args>
info(Args&&...) -> info<Args...>;

template <typename... Args>
struct [[maybe_unused]] warning {
  warning(Args&&... args,
          const source_location& loc = source_location::current())
  {
    global_logger()->log(loc, level::warning, std::forward<Args>(args)...);
  }
};

template <typename... Args>
warning(Args&&...) -> warning<Args...>;

template <typename... Args>
struct [[maybe_unused]] error {
  error(Args&&... args, const source_location& loc = source_location::current())
  {
    global_logger()->log(loc, level::error, std::forward<Args>(args)...);
  }
};

template <typename... Args>
error(Args&&...) -> error<Args...>;

template <typename... Args>
struct [[maybe_unused]] fatal {
  fatal(Args&&... args, const source_location& loc = source_location::current())
  {
    global_logger()->log(loc, level::fatal, std::forward<Args>(args)...);
  }
};

template <typename... Args>
fatal(Args&&...) -> fatal<Args...>;

}

FITORIA_NAMESPACE_END

#endif
