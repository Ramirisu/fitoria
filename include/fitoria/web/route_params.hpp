//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ROUTE_PARAMS_HPP
#define FITORIA_WEB_ROUTE_PARAMS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>
#include <fitoria/core/unordered_string_map.hpp>

#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

class route_params {
public:
  using map_type = unordered_string_map<std::string>;
  using keys_type = std::vector<std::string>;

  class const_iterator_impl {
    friend class route_params;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename map_type::value_type;
    using reference = typename map_type::const_reference;
    using pointer = typename map_type::const_pointer;
    using difference_type = typename map_type::difference_type;

    reference operator*() const noexcept
    {
      return *parent_->find(*key_iter_);
    }

    pointer operator->() const noexcept
    {
      return &*parent_->find(*key_iter_);
    }

    const_iterator_impl& operator++() noexcept
    {
      ++key_iter_;
      return *this;
    }

    const_iterator_impl operator++(int) noexcept
    {
      auto it = *this;
      ++*this;
      return it;
    }

    const_iterator_impl& operator--() noexcept
    {
      --key_iter_;
      return *this;
    }

    const_iterator_impl operator--(int) noexcept
    {
      auto it = *this;
      --*this;
      return it;
    }

    friend bool operator==(const const_iterator_impl& lhs,
                           const const_iterator_impl& rhs) noexcept
        = default;

  private:
    const_iterator_impl() = default;

    const_iterator_impl(const route_params& parent,
                        typename keys_type::const_iterator key_iter)
        : parent_(&parent)
        , key_iter_(std::move(key_iter))
    {
    }

    const route_params* parent_ = nullptr;
    typename keys_type::const_iterator key_iter_;
  };

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
  using iterator = const_iterator_impl;
  using const_iterator = const_iterator_impl;

  route_params() = default;

  route_params(std::string match_pattern,
               std::string match_path,
               std::initializer_list<value_type> params)
      : match_pattern_(std::move(match_pattern))
      , match_path_(std::move(match_path))
  {
    for (auto& [key, value] : params) {
      keys_.push_back(key);
      map_[key] = value;
    }
  }

  template <typename Key, typename Value>
    requires std::constructible_from<key_type, Key>
                 && std::constructible_from<mapped_type, Value>
  route_params(std::string match_pattern,
               std::string match_path,
               std::vector<std::pair<Key, Value>> params)
      : match_pattern_(std::move(match_pattern))
      , match_path_(std::move(match_path))
  {
    for (auto& [key, value] : params) {
      keys_.push_back(key);
      map_[key] = value;
    }
  }

  route_params(const route_params&) = default;

  route_params(route_params&&) = default;

  route_params& operator=(const route_params&) = default;

  route_params& operator=(route_params&&) = default;

  const std::string& match_pattern() const noexcept
  {
    return match_pattern_;
  }

  const std::string& match_path() const noexcept
  {
    return match_path_;
  }

  keys_type keys() const
  {
    return keys_;
  }

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

  bool contains(const std::string& name) const noexcept
  {
    return map_.contains(name);
  }

  template <typename Key>
    requires(!std::integral<Key>)
  optional<const mapped_type&> get(Key&& key) const noexcept
  {
    if (auto it = map_.find(std::forward<Key>(key)); it != map_.end()) {
      return it->second;
    }

    return nullopt;
  }

  optional<const mapped_type&> get(std::size_t index) const noexcept
  {
    if (index < size()) {
      return get(keys_[index]);
    }

    return nullopt;
  }

  const mapped_type& at(const std::string& name) const
  {
    return map_.at(name);
  }

  const mapped_type& at(std::size_t index) const
  {
    return map_.at(keys_.at(index));
  }

  map_type::const_iterator find(const std::string& name) const
  {
    return map_.find(name);
  }

  map_type::const_iterator find(std::size_t index) const
  {
    if (index < size()) {
      return map_.find(keys_.at(index));
    }

    return map_.end();
  }

  auto begin() noexcept
  {
    return iterator(*this, keys_.cbegin());
  }

  auto begin() const noexcept
  {
    return const_iterator(*this, keys_.cbegin());
  }

  auto cbegin() const noexcept
  {
    return const_iterator(*this, keys_.cbegin());
  }

  auto end() noexcept
  {
    return iterator(*this, keys_.cend());
  }

  auto end() const noexcept
  {
    return const_iterator(*this, keys_.cend());
  }

  auto cend() const noexcept
  {
    return const_iterator(*this, keys_.cend());
  }

  friend bool operator==(const route_params&, const route_params&) = default;

private:
  std::string match_pattern_;
  std::string match_path_;
  keys_type keys_;
  map_type map_;
};

}

FITORIA_NAMESPACE_END

#endif
