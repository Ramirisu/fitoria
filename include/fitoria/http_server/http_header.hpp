//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>

#include <fitoria/http_server/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

class http_header : public query_map {
public:
  using query_map::at;
  using query_map::contains;
  using query_map::erase;
  using query_map::get;
  using query_map::set;

  http_header() = default;

  http_header(const http_header&) = default;

  http_header(http_header&&) = default;

  http_header& operator=(const http_header&) = default;

  http_header& operator=(http_header&&) = default;

  void set(http::field name, std::string value)
  {
    set(std::string(to_string(name)), std::move(value));
  }

  optional<mapped_type&> get(http::field name) noexcept
  {
    return get(to_string(name));
  }

  optional<const mapped_type&> get(http::field name) const noexcept
  {
    return get(to_string(name));
  }

  bool erase(http::field name)
  {
    return erase(to_string(name));
  }

  mapped_type& at(http::field name)
  {
    return at(to_string(name));
  }

  const mapped_type& at(http::field name) const
  {
    return at(to_string(name));
  }

  mapped_type& operator[](http::field name)
  {
    return static_cast<query_map&>(*this)[to_string(name)];
  }

  bool contains(http::field name) const
  {
    return contains(to_string(name));
  }
};

FITORIA_NAMESPACE_END
