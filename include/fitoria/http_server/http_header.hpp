//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/http_server/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

class http_header : public query_map {
public:
  using query_map::at;
  using query_map::contains;
  using query_map::erase;
  using query_map::get;
  using query_map::set;

  void set(field name, std::string value)
  {
    set(std::string(to_string(name)), std::move(value));
  }

  optional<mapped_type&> get(field name) noexcept
  {
    return get(to_string(name));
  }

  optional<const mapped_type&> get(field name) const noexcept
  {
    return get(to_string(name));
  }

  bool erase(field name)
  {
    return erase(to_string(name));
  }

  mapped_type& at(field name)
  {
    return at(to_string(name));
  }

  const mapped_type& at(field name) const
  {
    return at(to_string(name));
  }

  mapped_type& operator[](field name) noexcept
  {
    return static_cast<query_map&>(*this)[to_string(name)];
  }

  bool contains(field name) const
  {
    return contains(to_string(name));
  }
};

FITORIA_NAMESPACE_END
