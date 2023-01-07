//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>
#include <fitoria/core/unordered_string_map.hpp>

FITORIA_NAMESPACE_BEGIN

class query_map {
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

  query_map() = default;

  query_map(const query_map&) = default;

  query_map(query_map&&) = default;

  query_map& operator=(const query_map&) = default;

  query_map& operator=(query_map&&) = default;

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

  template <typename Key>
  optional<mapped_type&> get(Key&& key) noexcept
  {
    if (auto it = map_.find(std::forward<Key>(key)); it != map_.end()) {
      return optional<mapped_type&>(it->second);
    }

    return nullopt;
  }

  template <typename Key>
  optional<const mapped_type&> get(Key&& key) const noexcept
  {
    if (auto it = map_.find(std::forward<Key>(key)); it != map_.end()) {
      return optional<const mapped_type&>(it->second);
    }

    return nullopt;
  }

  bool erase(const std::string& name)
  {
    return map_.erase(name) == 1;
  }

  mapped_type& at(const std::string& name)
  {
    return map_.at(name);
  }

  const mapped_type& at(const std::string& name) const
  {
    return map_.at(name);
  }

  mapped_type& operator[](const std::string& name)
  {
    return map_[name];
  }

  bool contains(const std::string& name) const
  {
    return map_.contains(name);
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
  map_type map_;
};

FITORIA_NAMESPACE_END
