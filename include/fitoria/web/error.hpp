//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ERROR_HPP
#define FITORIA_WEB_ERROR_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/error.hpp>

#include <type_traits>

FITORIA_NAMESPACE_BEGIN

namespace web {

enum class error {
  route_parse_error,
  route_already_exists,
  route_not_exists,
  unexpected_content_type,
};

class error_category : public std::error_category {
public:
  ~error_category() override = default;

  const char* name() const noexcept override
  {
    return "fitoria.error";
  }

  std::string message(int condition) const override
  {
    switch (static_cast<error>(condition)) {
    case error::route_parse_error:
      return "the route being parsed is invalid";
    case error::route_already_exists:
      return "the route being registered already exists";
    case error::route_not_exists:
      return "the route being searched doesn't exist";
    case error::unexpected_content_type:
      return "unexpected Content-Type";
    default:
      break;
    }

    return {};
  }
};

inline error_code make_error_code(error e)
{
  static const error_category c;
  return error_code(static_cast<int>(e), c);
}

}

FITORIA_NAMESPACE_END

namespace std {

template <>
struct is_error_code_enum<FITORIA_NAMESPACE::web::error> : std::true_type { };

}

#endif
