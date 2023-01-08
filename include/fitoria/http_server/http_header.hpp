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
  using base_type = query_map;

  base_type& base() noexcept
  {
    return static_cast<base_type&>(*this);
  }

  const base_type& base() const noexcept
  {
    return static_cast<const base_type&>(*this);
  }

public:
  http_header() = default;

  http_header(const http_header&) = default;

  http_header(http_header&&) = default;

  http_header& operator=(const http_header&) = default;

  http_header& operator=(http_header&&) = default;

  void set(std::string name, std::string value)
  {
    normalize_field(name);
    base().set(std::move(name), std::move(value));
  }

  void set(http::field name, std::string value)
  {
    base().set(std::string(to_string(name)), std::move(value));
  }

  optional<mapped_type&> get(std::string name) noexcept
  {
    normalize_field(name);
    return base().get(std::move(name));
  }

  optional<mapped_type&> get(http::field name) noexcept
  {
    return base().get(to_string(name));
  }

  optional<const mapped_type&> get(std::string name) const noexcept
  {
    normalize_field(name);
    return base().get(std::move(name));
  }

  optional<const mapped_type&> get(http::field name) const noexcept
  {
    return base().get(to_string(name));
  }

  bool erase(std::string name)
  {
    normalize_field(name);
    return base().erase(std::move(name));
  }

  bool erase(http::field name)
  {
    return base().erase(to_string(name));
  }

  mapped_type& at(std::string name)
  {
    normalize_field(name);
    return base().at(std::move(name));
  }

  mapped_type& at(http::field name)
  {
    return base().at(to_string(name));
  }

  const mapped_type& at(std::string name) const
  {
    normalize_field(name);
    return base().at(std::move(name));
  }

  const mapped_type& at(http::field name) const
  {
    return base().at(to_string(name));
  }

  mapped_type& operator[](std::string name)
  {
    normalize_field(name);
    return base().operator[](std::move(name));
  }

  mapped_type& operator[](http::field name)
  {
    return base().operator[](to_string(name));
  }

  bool contains(std::string name) const
  {
    normalize_field(name);
    return base().contains(std::move(name));
  }

  bool contains(http::field name) const
  {
    return base().contains(to_string(name));
  }

private:
  static void normalize_field(std::string& name) noexcept
  {
    bool upper = true;
    for (std::size_t i = 0; i < name.size(); ++i) {
      if (name[i] == '-') {
        upper = true;
      } else if (upper) {
        name[i] = static_cast<char>(::toupper(name[i]));
        upper = false;
      } else {
        name[i] = static_cast<char>(::tolower(name[i]));
      }
    }
  }
};

FITORIA_NAMESPACE_END
