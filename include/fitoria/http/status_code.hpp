//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_STATUS_CODE_HPP
#define FITORIA_HTTP_STATUS_CODE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>
#include <fitoria/core/utility.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http {

using boost::beast::http::status;
using boost::beast::http::status_class;

class status_code {
public:
  status_code(status value)
      : value_(value)
  {
  }

  status_code(int value)
      : value_(static_cast<status>(value))
  {
  }

  auto value() const noexcept -> status
  {
    return value_;
  }

  auto category() const noexcept -> status_class
  {
    return to_status_class(value_);
  }

  friend constexpr bool operator==(const status_code&, const status_code&)
      = default;

private:
  status value_;
};

}

FITORIA_NAMESPACE_END

#include <fitoria/core/format.hpp>

template <>
struct FITORIA_NAMESPACE::fmt::formatter<FITORIA_NAMESPACE::http::status_code>
    : FITORIA_NAMESPACE::fmt::formatter<
          std::underlying_type_t<FITORIA_NAMESPACE::http::status>> {
  template <typename FormatContext>
  auto format(FITORIA_NAMESPACE::http::status_code status_code,
              FormatContext& ctx) const
  {
    return FITORIA_NAMESPACE::fmt::formatter<
        std::underlying_type_t<FITORIA_NAMESPACE::http::status>>::
        format(FITORIA_NAMESPACE::to_underlying(status_code.value()), ctx);
  }
};

#endif
