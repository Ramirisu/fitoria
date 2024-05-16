//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CLIENT_ERROR_HPP
#define FITORIA_CLIENT_ERROR_HPP

#include <fitoria/core/config.hpp>

#include <system_error>

FITORIA_NAMESPACE_BEGIN

namespace client {

enum class error {
  content_type_not_application_json,
};

class error_category : public std::error_category {
public:
  ~error_category() override = default;

  const char* name() const noexcept override
  {
    return "fitoria.client.error";
  }

  std::string message(int condition) const override
  {
    switch (static_cast<error>(condition)) {
    case error::content_type_not_application_json:
      return "unexpected Content-Type received, expected \"Content-Type: "
             "application/json\"";
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
struct std::is_error_code_enum<FITORIA_NAMESPACE::client::error>
    : std::true_type { };

#endif
