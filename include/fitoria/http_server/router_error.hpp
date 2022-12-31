//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/error.hpp>

#include <type_traits>

FITORIA_NAMESPACE_BEGIN

enum class router_error {
  route_parse_error,
  route_already_exists,
  route_not_exists,
};

class router_error_category : public std::error_category {
public:
  const char* name() const noexcept override
  {
    return "fitoria.router_error";
  }

  std::string message(int condition) const override
  {
    switch (static_cast<router_error>(condition)) {
    case router_error::route_parse_error:
      return "the route being parsed is invalid";
    case router_error::route_already_exists:
      return "the route being registered already exists";
    case router_error::route_not_exists:
      return "the route being searched doesn't exist";
    default:
      break;
    }

    return {};
  }
};

error_code make_error_code(router_error e)
{
  static const router_error_category c;
  return error_code(static_cast<int>(e), c);
}

FITORIA_NAMESPACE_END

namespace std {

template <>
struct is_error_code_enum<fitoria::router_error> : std::true_type { };
}