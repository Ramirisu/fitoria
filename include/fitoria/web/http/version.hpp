//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HTTP_VERSION_HPP
#define FITORIA_WEB_HTTP_VERSION_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>

#include <fitoria/web/http/http.hpp>

#include <algorithm>

FITORIA_NAMESPACE_BEGIN

namespace web::http {

enum class version : std::uint8_t {
  unknown,
  v0_9,
  v1_0,
  v1_1,
  v2_0,
};

inline std::string to_string(version ver)
{
  switch (ver) {
  case version::v0_9:
    return "0.9";
  case version::v1_0:
    return "1.0";
  case version::v1_1:
    return "1.1";
  case version::v2_0:
    return "2.0";
  default:
    break;
  }

  return "unknown";
}

inline version to_version(unsigned int ver)
{
  if (ver == 9) {
    return version::v0_9;
  }
  if (ver == 10) {
    return version::v1_0;
  }
  if (ver == 11) {
    return version::v1_1;
  }
  if (ver == 20) {
    return version::v2_0;
  }

  return version::unknown;
}

}

FITORIA_NAMESPACE_END

template <typename CharT>
struct FITORIA_NAMESPACE::fmt::formatter<FITORIA_NAMESPACE::web::http::version,
                                         CharT> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    auto it = ctx.begin();
    if (it == ctx.end()) {
      return it;
    }
    if (*it != '}') {
      FITORIA_THROW_OR(
          FITORIA_NAMESPACE::fmt::format_error(
              "invalid format args for fitoria::web::http::version"),
          std::terminate());
    }

    return it;
  }

  template <typename FormatContext>
  auto format(FITORIA_NAMESPACE::web::http::version ver,
              FormatContext& ctx) const
  {
    auto str = to_string(ver);
    return std::copy(str.begin(), str.end(), ctx.out());
  }
};

#endif
