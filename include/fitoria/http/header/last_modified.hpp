//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_HEADER_LAST_MODIFIED_HPP
#define FITORIA_HTTP_HEADER_LAST_MODIFIED_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/chrono.hpp>
#include <fitoria/core/format.hpp>

#include <chrono>
#include <string>

FITORIA_NAMESPACE_BEGIN

namespace http::header {

inline auto last_modified(std::chrono::time_point<chrono::utc_clock> time)

    -> std::string
{
  // https://datatracker.ietf.org/doc/html/rfc2616#section-14.29
  // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Last-Modified
  return fmt::format("{:%a, %d %b %Y %T} GMT",
                     std::chrono::floor<std::chrono::seconds>(time));
}

}

FITORIA_NAMESPACE_END

#endif
