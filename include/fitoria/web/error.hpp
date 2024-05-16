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
  content_type_not_application_form_urlencoded,
  content_type_not_application_json,
  extractor_field_count_not_match,
  extractor_field_name_not_found,
  state_not_found,
  not_upgrade,
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
    case error::content_type_not_application_form_urlencoded:
      // clang-format off
      return "unexpected Content-Type received, expected \"Content-Type: application/x-www-form-urlencoded\"";
      // clang-format on
    case error::content_type_not_application_json:
      // clang-format off
      return "unexpected Content-Type received, expected \"Content-Type: application/json\"";
      // clang-format on
    case error::extractor_field_count_not_match:
      // clang-format off
      return "unable to extract key/value into struct, field count not match";
      // clang-format on
    case error::extractor_field_name_not_found:
      // clang-format off
      return "unable to extract key/value into struct, field not found";
      // clang-format on
    case error::state_not_found:
      return "the state being obtained doesn't exist";
    case error::not_upgrade:
      return "request is not upgradable";
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
