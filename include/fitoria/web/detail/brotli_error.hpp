//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_DETAIL_BROTLI_ERROR_HPP
#define FITORIA_WEB_DETAIL_BROTLI_ERROR_HPP

#include <fitoria/core/config.hpp>

#include <system_error>

FITORIA_NAMESPACE_BEGIN

namespace web::detail {

enum class brotli_error {
  error = 1,
  need_more_input = 2,
  need_more_output = 3,

  init = 99,
  unknown = 100,
};

class brotli_error_category : public std::error_category {
public:
  ~brotli_error_category() override = default;

  const char* name() const noexcept override
  {
    return "fitoria.web.brotli_error";
  }

  std::string message(int condition) const override
  {
    switch (static_cast<brotli_error>(condition)) {
    case brotli_error::error:
      return "decode error";
    case brotli_error::need_more_input:
      return "need more input";
    case brotli_error::need_more_output:
      return "need more output";
    case brotli_error::init:
      return "initialzation error";
    case brotli_error::unknown:
      return "unknown error";
    default:
      break;
    }

    return {};
  }
};

inline std::error_code make_error_code(brotli_error e)
{
  static const brotli_error_category c;
  return { static_cast<int>(e), c };
}
}

FITORIA_NAMESPACE_END

template <>
struct std::is_error_code_enum<FITORIA_NAMESPACE::web::detail::brotli_error>
    : std::true_type { };

#endif
