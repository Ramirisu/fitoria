
//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/json.hpp>

#include <fitoria/http_server/http_context.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename T>
class from_json {
public:
  from_json(const http_context& c)
      : value_(json::value_to<T>(json::parse(c.request().body())))
  {
  }

  T& value() & noexcept
  {
    return value_;
  }

  const T& value() const& noexcept
  {
    return value_;
  }

  T&& value() && noexcept
  {
    return std::move(value_);
  }

  const T&& value() const&& noexcept
  {
    return std::move(value_);
  }

private:
  T value_;
};

FITORIA_NAMESPACE_END
