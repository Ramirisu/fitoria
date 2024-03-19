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

#include <fitoria/log/registry.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

template <typename... Args>
class log {
public:
  log(level lv,
      fmt::format_string<Args...> fmt,
      Args&&... args,
      const source_location& loc = source_location::current())
  {
    if (auto logger = registry::global().default_logger(); logger) {
      logger->log(loc, lv, fmt, std::forward<Args>(args)...);
    }
  }
};

template <typename... Args>
log(level, fmt::format_string<Args...>, Args&&...) -> log<Args...>;

template <typename... Args>
class trace {
public:
  trace(fmt::format_string<Args...> fmt,
        Args&&... args,
        const source_location& loc = source_location::current())
  {
    if (auto logger = registry::global().default_logger(); logger) {
      logger->log(loc, level::trace, fmt, std::forward<Args>(args)...);
    }
  }
};

template <typename... Args>
trace(fmt::format_string<Args...>, Args&&...) -> trace<Args...>;

template <typename... Args>
class debug {
public:
  debug(fmt::format_string<Args...> fmt,
        Args&&... args,
        const source_location& loc = source_location::current())
  {
    if (auto logger = registry::global().default_logger(); logger) {
      logger->log(loc, level::debug, fmt, std::forward<Args>(args)...);
    }
  }
};

template <typename... Args>
debug(fmt::format_string<Args...>, Args&&...) -> debug<Args...>;

template <typename... Args>
class info {
public:
  info(fmt::format_string<Args...> fmt,
       Args&&... args,
       const source_location& loc = source_location::current())
  {
    if (auto logger = registry::global().default_logger(); logger) {
      logger->log(loc, level::info, fmt, std::forward<Args>(args)...);
    }
  }
};

template <typename... Args>
info(fmt::format_string<Args...>, Args&&...) -> info<Args...>;

template <typename... Args>
class warning {
public:
  warning(fmt::format_string<Args...> fmt,
          Args&&... args,
          const source_location& loc = source_location::current())
  {
    if (auto logger = registry::global().default_logger(); logger) {
      logger->log(loc, level::warning, fmt, std::forward<Args>(args)...);
    }
  }
};

template <typename... Args>
warning(fmt::format_string<Args...>, Args&&...) -> warning<Args...>;

template <typename... Args>
class error {
public:
  error(fmt::format_string<Args...> fmt,
        Args&&... args,
        const source_location& loc = source_location::current())
  {
    if (auto logger = registry::global().default_logger(); logger) {
      logger->log(loc, level::error, fmt, std::forward<Args>(args)...);
    }
  }
};

template <typename... Args>
error(fmt::format_string<Args...>, Args&&...) -> error<Args...>;

template <typename... Args>
class fatal {
public:
  fatal(fmt::format_string<Args...> fmt,
        Args&&... args,
        const source_location& loc = source_location::current())
  {
    if (auto logger = registry::global().default_logger(); logger) {
      logger->log(loc, level::fatal, fmt, std::forward<Args>(args)...);
    }
  }
};

template <typename... Args>
fatal(fmt::format_string<Args...>, Args&&...) -> fatal<Args...>;

}

FITORIA_NAMESPACE_END

#endif
