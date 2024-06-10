//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ERROR_HPP
#define FITORIA_WEB_ERROR_HPP

#include <fitoria/core/config.hpp>

#include <system_error>

FITORIA_NAMESPACE_BEGIN

namespace web {

enum class error {
  route_already_exists,
  route_not_exists,
  unexpected_content_type,
  extractor_field_name_not_found,
};

class error_category : public std::error_category {
public:
  ~error_category() override = default;

  const char* name() const noexcept override
  {
    return "fitoria.web.error";
  }

  std::string message(int condition) const override
  {
    switch (static_cast<error>(condition)) {
    case error::route_already_exists:
      return "the route being registered already exists";
    case error::route_not_exists:
      return "the route being searched doesn't exist";
    case error::unexpected_content_type:
      return R"(unexpected "Content-Type".)";
    case error::extractor_field_name_not_found:
      return "unable to extract key/value into struct, field not found";
    default:
      break;
    }

    return {};
  }
};

inline std::error_code make_error_code(error e)
{
  static const error_category c;
  return { static_cast<int>(e), c };
}

}

FITORIA_NAMESPACE_END

template <>
struct std::is_error_code_enum<FITORIA_NAMESPACE::web::error> : std::true_type {
};

#endif
