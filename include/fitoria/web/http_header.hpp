//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>
#include <fitoria/core/optional.hpp>
#include <fitoria/core/unordered_string_map.hpp>

FITORIA_NAMESPACE_BEGIN

class http_header {
public:
  using map_type = unordered_string_map<std::string>;

public:
  using key_type = typename map_type::key_type;
  using mapped_type = typename map_type::mapped_type;
  using value_type = typename map_type::value_type;
  using size_type = typename map_type::size_type;
  using difference_type = typename map_type::difference_type;
  using reference = typename map_type::reference;
  using const_reference = typename map_type::const_reference;
  using pointer = typename map_type::pointer;
  using const_pointer = typename map_type::const_pointer;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

  http_header() = default;

  http_header(const http_header&) = default;

  http_header(http_header&&) = default;

  http_header& operator=(const http_header&) = default;

  http_header& operator=(http_header&&) = default;

  bool empty() const noexcept
  {
    return map_.empty();
  }

  size_type size() const noexcept
  {
    return map_.size();
  }

  size_type max_size() const noexcept
  {
    return map_.max_size();
  }

  void clear() noexcept
  {
    map_.clear();
  }

  void set(std::string name, std::string value)
  {
    normalize_field(name);
    map_.insert({ std::move(name), std::move(value) });
  }

  void set(http::field name, std::string value)
  {
    set(std::string(to_string(name)), std::move(value));
  }

  optional<mapped_type&> get(std::string name) noexcept
  {
    normalize_field(name);
    if (auto it = map_.find(name); it != map_.end()) {
      return it->second;
    }

    return nullopt;
  }

  optional<mapped_type&> get(http::field name) noexcept
  {
    return get(to_string(name));
  }

  optional<const mapped_type&> get(std::string name) const noexcept
  {
    normalize_field(name);
    if (auto it = map_.find(name); it != map_.end()) {
      return it->second;
    }

    return nullopt;
  }

  optional<const mapped_type&> get(http::field name) const noexcept
  {
    return get(to_string(name));
  }

  bool erase(std::string name)
  {
    normalize_field(name);
    return map_.erase(std::move(name));
  }

  bool erase(http::field name)
  {
    return map_.erase(to_string(name));
  }

  mapped_type& at(std::string name)
  {
    normalize_field(name);
    return map_.at(std::move(name));
  }

  mapped_type& at(http::field name)
  {
    return map_.at(to_string(name));
  }

  const mapped_type& at(std::string name) const
  {
    normalize_field(name);
    return map_.at(std::move(name));
  }

  const mapped_type& at(http::field name) const
  {
    return map_.at(to_string(name));
  }

  mapped_type& operator[](std::string name)
  {
    normalize_field(name);
    return map_.operator[](std::move(name));
  }

  mapped_type& operator[](http::field name)
  {
    return map_.operator[](to_string(name));
  }

  bool contains(std::string name) const
  {
    normalize_field(name);
    return map_.contains(std::move(name));
  }

  bool contains(http::field name) const
  {
    return map_.contains(to_string(name));
  }

  auto begin() noexcept
  {
    return map_.begin();
  }

  auto begin() const noexcept
  {
    return map_.begin();
  }

  auto cbegin() const noexcept
  {
    return map_.cbegin();
  }

  auto end() noexcept
  {
    return map_.end();
  }

  auto end() const noexcept
  {
    return map_.end();
  }

  auto cend() const noexcept
  {
    return map_.cend();
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

private:
  map_type map_;
};

FITORIA_NAMESPACE_END
