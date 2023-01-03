//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>

#include <chrono>
#include <iostream>
#include <source_location>

FITORIA_NAMESPACE_BEGIN

namespace log {

namespace detail {

  enum class level {
    debug,
    info,
    warning,
    error,
    fatal,
    off,
  };

  std::string_view to_string(level lv)
  {
    switch (lv) {
    case level::debug:
      return "DEBUG";
    case level::info:
      return "INFO";
    case level::warning:
      return "WARNING";
    case level::error:
      return "ERROR";
    case level::fatal:
      return "FATAL";
    default:
      break;
    }

    return "UNKNOWN";
  }

  void sink(std::string_view msg)
  {
    std::cout << msg;
  }

  template <typename Format, typename... Args>
  void log(const std::source_location& loc,
           level lv,
           std::string_view caller,
           Format&& fmt,
           Args&&... args)
  {
    sink(fmt::format("[{:%FT%TZ} {} {}] ",
                     std::chrono::floor<std::chrono::seconds>(
                         std::chrono::system_clock::now()),
                     to_string(lv), caller));
    sink(fmt::vformat(std::forward<Format>(fmt),
                      fmt::make_format_args(std::forward<Args>(args)...)));
    sink(fmt::format(" [{}:{}:{}]\n", loc.file_name(), loc.line(),
                     loc.column()));
  }

}

template <typename Format, typename... Args>
struct debug {
  debug(std::string_view caller,
        Format&& fmt,
        Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    detail::log(loc, detail::level::debug, caller, std::forward<Format>(fmt),
                std::forward<Args>(args)...);
  }
};

template <typename... Args>
debug(std::string_view, Args&&...) -> debug<Args...>;

template <typename Format, typename... Args>
struct info {
  info(std::string_view caller,
       Format&& fmt,
       Args&&... args,
       const std::source_location& loc = std::source_location::current())
  {
    detail::log(loc, detail::level::info, caller, std::forward<Format>(fmt),
                std::forward<Args>(args)...);
  }
};

template <typename... Args>
info(std::string_view, Args&&...) -> info<Args...>;

template <typename Format, typename... Args>
struct warning {
  warning(std::string_view caller,
          Format&& fmt,
          Args&&... args,
          const std::source_location& loc = std::source_location::current())
  {
    detail::log(loc, detail::level::warning, caller, std::forward<Format>(fmt),
                std::forward<Args>(args)...);
  }
};

template <typename... Args>
warning(std::string_view, Args&&...) -> warning<Args...>;

template <typename Format, typename... Args>
struct error {
  error(std::string_view caller,
        Format&& fmt,
        Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    detail::log(loc, detail::level::error, caller, std::forward<Format>(fmt),
                std::forward<Args>(args)...);
  }
};

template <typename... Args>
error(std::string_view, Args&&...) -> error<Args...>;

template <typename Format, typename... Args>
struct fatal {
  fatal(std::string_view caller,
        Format&& fmt,
        Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    detail::log(loc, detail::level::fatal, caller, std::forward<Format>(fmt),
                std::forward<Args>(args)...);
  }
};

template <typename... Args>
fatal(std::string_view, Args&&...) -> fatal<Args...>;

}

FITORIA_NAMESPACE_END
