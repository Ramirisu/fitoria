//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_STATUS_CODE_HPP
#define FITORIA_WEB_HTTP_STATUS_CODE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>

#include <fitoria/web/http/http.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::http {

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

  status value() const noexcept
  {
    return value_;
  }

  status_class category() const noexcept
  {
    return to_status_class(value_);
  }

  optional<status> match(std::initializer_list<status> list)
  {
    for (auto& sc : list) {
      if (sc == value_) {
        return value_;
      }
    }

    return nullopt;
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
struct FITORIA_NAMESPACE::fmt::formatter<
    FITORIA_NAMESPACE::web::http::status_code>
    : FITORIA_NAMESPACE::fmt::formatter<
          std::underlying_type_t<FITORIA_NAMESPACE::web::http::status>> {
  template <typename FormatContext>
  auto format(FITORIA_NAMESPACE::web::http::status_code status_code,
              FormatContext& ctx) const
  {
    return FITORIA_NAMESPACE::fmt::formatter<
        std::underlying_type_t<FITORIA_NAMESPACE::web::http::status>>::
        format(FITORIA_NAMESPACE::to_underlying(status_code.value()), ctx);
  }
};

#endif
