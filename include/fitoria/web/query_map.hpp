//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_QUERY_MAP_HPP
#define FITORIA_WEB_QUERY_MAP_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>
#include <fitoria/core/unordered_string_map.hpp>
#include <fitoria/core/url.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

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

  query_map(std::initializer_list<value_type> init)
      : map_(std::move(init))
  {
  }

  query_map(const query_map&) = default;

  query_map(query_map&&) = default;

  query_map& operator=(const query_map&) = default;

  query_map& operator=(query_map&&) = default;

  auto empty() const noexcept -> bool
  {
    return map_.empty();
  }

  auto size() const noexcept -> size_type
  {
    return map_.size();
  }

  auto max_size() const noexcept -> size_type
  {
    return map_.max_size();
  }

  auto contains(const std::string& name) const noexcept -> bool
  {
    return map_.contains(name);
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
  auto get(Key&& key) noexcept -> optional<mapped_type&>
  {
    if (auto it = map_.find(std::forward<Key>(key)); it != map_.end()) {
      return it->second;
    }

    return nullopt;
  }

  template <typename Key>
  auto get(Key&& key) const noexcept -> optional<const mapped_type&>
  {
    if (auto it = map_.find(std::forward<Key>(key)); it != map_.end()) {
      return it->second;
    }

    return nullopt;
  }

  auto erase(const std::string& name) -> optional<mapped_type>
  {
    if (auto it = map_.find(name); it != map_.end()) {
      auto value = std::move(it->second);
      map_.erase(it);
      return value;
    }

    return nullopt;
  }

  auto at(const std::string& name) -> mapped_type&
  {
    return map_.at(name);
  }

  auto at(const std::string& name) const -> const mapped_type&
  {
    return map_.at(name);
  }

  auto operator[](const std::string& name) -> mapped_type&
  {
    return map_[name];
  }

  auto to_string() const -> std::string
  {
    std::string query;
    for (auto& [name, value] : map_) {
      query += name;
      query += "=";
      query += value;
      query += "&";
    }
    if (!query.empty()) {
      query.pop_back();
    }

    return query;
  }

  auto begin() noexcept -> iterator
  {
    return map_.begin();
  }

  auto begin() const noexcept -> const_iterator
  {
    return map_.begin();
  }

  auto cbegin() const noexcept -> const_iterator
  {
    return map_.cbegin();
  }

  auto end() noexcept -> iterator
  {
    return map_.end();
  }

  auto end() const noexcept -> const_iterator
  {
    return map_.end();
  }

  auto cend() const noexcept -> const_iterator
  {
    return map_.cend();
  }

  friend bool operator==(const query_map&, const query_map&) = default;

  static auto from(boost::urls::params_view params) -> query_map
  {
    query_map query;
    for (auto param : params) {
      if (param.has_value) {
        query[param.key] = param.value;
      }
    }

    return query;
  }

private:
  map_type map_;
};
}

FITORIA_NAMESPACE_END

#endif
