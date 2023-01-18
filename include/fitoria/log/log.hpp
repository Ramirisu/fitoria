//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_LOG_LOG_HPP
#define FITORIA_LOG_LOG_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/log/logger.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

template <typename... Args>
struct [[maybe_unused]] debug {
  debug(Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::debug, std::forward<Args>(args)...);
  }
};

template <typename... Args>
debug(Args&&...) -> debug<Args...>;

template <typename... Args>
struct [[maybe_unused]] info {
  info(Args&&... args,
       const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::info, std::forward<Args>(args)...);
  }
};

template <typename... Args>
info(Args&&...) -> info<Args...>;

template <typename... Args>
struct [[maybe_unused]] warning {
  warning(Args&&... args,
          const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::warning, std::forward<Args>(args)...);
  }
};

template <typename... Args>
warning(Args&&...) -> warning<Args...>;

template <typename... Args>
struct [[maybe_unused]] error {
  error(Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::error, std::forward<Args>(args)...);
  }
};

template <typename... Args>
error(Args&&...) -> error<Args...>;

template <typename... Args>
struct [[maybe_unused]] fatal {
  fatal(Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::fatal, std::forward<Args>(args)...);
  }
};

template <typename... Args>
fatal(Args&&...) -> fatal<Args...>;

}

FITORIA_NAMESPACE_END

#endif
