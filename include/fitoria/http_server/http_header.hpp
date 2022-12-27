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

FITORIA_NAMESPACE_BEGIN

class http_header {
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
    map_.insert({ std::move(name), std::move(value) });
  }

  void set(field name, std::string value)
  {
    map_.insert({ std::string(to_string(name)), std::move(value) });
  }

  bool erase(const std::string& name)
  {
    return map_.erase(name) == 1;
  }

  bool erase(field name)
  {
    return map_.erase(to_string(name)) == 1;
  }

  mapped_type& at(const std::string& name)
  {
    return map_.at(name);
  }

  const mapped_type& at(const std::string& name) const
  {
    return map_.at(name);
  }

  mapped_type& at(field name)
  {
    return map_.at(to_string(name));
  }

  const mapped_type& at(field name) const
  {
    return map_.at(to_string(name));
  }

  mapped_type& operator[](const std::string& name) noexcept
  {
    return map_[name];
  }

  mapped_type& operator[](field name) noexcept
  {
    return map_[to_string(name)];
  }

  bool contains(const std::string& name) const
  {
    return map_.contains(name);
  }

  bool contains(field name) const
  {
    return map_.contains(to_string(name));
  }

  auto begin()
  {
    return map_.begin();
  }

  auto begin() const
  {
    return map_.begin();
  }

  auto cbegin()
  {
    return map_.cbegin();
  }

  auto end()
  {
    return map_.end();
  }

  auto end() const
  {
    return map_.end();
  }

  auto cend()
  {
    return map_.cend();
  }

private:
  unordered_string_map<std::string> map_;
};

FITORIA_NAMESPACE_END