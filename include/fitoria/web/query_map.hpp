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

/// @verbatim embed:rst:leading-slashes
///
/// A type for dealing with query string parameters.
///
/// @endverbatim
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

  /// @verbatim embed:rst:leading-slashes
  ///
  /// The default constructor creates an empty map.
  ///
  /// @endverbatim
  query_map() = default;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct the map by a list of key/value pairs.
  ///
  /// @endverbatim
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

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the number of key/value pairs.
  ///
  /// @endverbatim
  auto size() const noexcept -> size_type
  {
    return map_.size();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the maximum possible number of key/value pairs.
  ///
  /// @endverbatim
  auto max_size() const noexcept -> size_type
  {
    return map_.max_size();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Checks if the container contains element with specific key.
  ///
  /// @endverbatim
  auto contains(const std::string& name) const noexcept -> bool
  {
    return map_.contains(name);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Clear the contents.
  ///
  /// @endverbatim
  void clear() noexcept
  {
    map_.clear();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Insert new element or update value of existing element with specific key.
  ///
  /// @endverbatim
  void set(std::string name, std::string value)
  {
    map_.insert({ std::move(name), std::move(value) });
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get value with specific key.
  ///
  /// @endverbatim
  template <typename Key>
  auto get(Key&& key) noexcept -> optional<mapped_type&>
  {
    if (auto it = map_.find(std::forward<Key>(key)); it != map_.end()) {
      return it->second;
    }

    return nullopt;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get value with specific key.
  ///
  /// @endverbatim
  template <typename Key>
  auto get(Key&& key) const noexcept -> optional<const mapped_type&>
  {
    if (auto it = map_.find(std::forward<Key>(key)); it != map_.end()) {
      return it->second;
    }

    return nullopt;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Remove element with specific key.
  ///
  /// @endverbatim
  auto erase(const std::string& name) -> optional<mapped_type>
  {
    if (auto it = map_.find(name); it != map_.end()) {
      auto value = std::move(it->second);
      map_.erase(it);
      return value;
    }

    return nullopt;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get value with specific key.
  ///
  /// @endverbatim
  auto at(const std::string& name) -> mapped_type&
  {
    return map_.at(name);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get value with specific key.
  ///
  /// @endverbatim
  auto at(const std::string& name) const -> const mapped_type&
  {
    return map_.at(name);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get or insert specific element.
  ///
  /// @endverbatim
  auto operator[](const std::string& name) -> mapped_type&
  {
    return map_[name];
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Converts key/value pairs to query string.
  ///
  /// @endverbatim
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

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning.
  ///
  /// @endverbatim
  auto begin() noexcept -> iterator
  {
    return map_.begin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning.
  ///
  /// @endverbatim
  auto begin() const noexcept -> const_iterator
  {
    return map_.begin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning.
  ///
  /// @endverbatim
  auto cbegin() const noexcept -> const_iterator
  {
    return map_.cbegin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end.
  ///
  /// @endverbatim
  auto end() noexcept -> iterator
  {
    return map_.end();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end.
  ///
  /// @endverbatim
  auto end() const noexcept -> const_iterator
  {
    return map_.end();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end.
  ///
  /// @endverbatim
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
