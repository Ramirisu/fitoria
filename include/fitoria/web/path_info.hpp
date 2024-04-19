//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_PATH_INFO_HPP
#define FITORIA_WEB_PATH_INFO_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>
#include <fitoria/core/unordered_string_map.hpp>

#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

class path_info {
public:
  using map_type = unordered_string_map<std::string>;
  using keys_type = std::vector<std::string>;

  class const_iterator_impl {
    friend class path_info;

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

    const_iterator_impl(const path_info& parent,
                        typename keys_type::const_iterator key_iter)
        : parent_(&parent)
        , key_iter_(std::move(key_iter))
    {
    }

    const path_info* parent_ = nullptr;
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

  path_info() = default;

  path_info(std::string match_pattern,
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
  path_info(std::string match_pattern,
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

  path_info(const path_info&) = default;

  path_info(path_info&&) = default;

  path_info& operator=(const path_info&) = default;

  path_info& operator=(path_info&&) = default;

  auto match_pattern() const noexcept -> const std::string&
  {
    return match_pattern_;
  }

  auto match_path() const noexcept -> const std::string&
  {
    return match_path_;
  }

  auto keys() const -> keys_type
  {
    return keys_;
  }

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

  template <typename Key>
    requires(!std::integral<Key>)
  auto get(Key&& key) const noexcept -> optional<const mapped_type&>
  {
    if (auto it = map_.find(std::forward<Key>(key)); it != map_.end()) {
      return it->second;
    }

    return nullopt;
  }

  auto get(std::size_t index) const noexcept -> optional<const mapped_type&>
  {
    if (index < size()) {
      return get(keys_[index]);
    }

    return nullopt;
  }

  auto at(const std::string& name) const -> const mapped_type&
  {
    return map_.at(name);
  }

  auto at(std::size_t index) const -> const mapped_type&
  {
    return map_.at(keys_.at(index));
  }

  auto find(const std::string& name) const -> map_type::const_iterator
  {
    return map_.find(name);
  }

  auto find(std::size_t index) const -> map_type::const_iterator
  {
    if (index < size()) {
      return map_.find(keys_.at(index));
    }

    return map_.end();
  }

  auto begin() noexcept -> iterator
  {
    return iterator(*this, keys_.cbegin());
  }

  auto begin() const noexcept -> const_iterator
  {
    return const_iterator(*this, keys_.cbegin());
  }

  auto cbegin() const noexcept -> const_iterator
  {
    return const_iterator(*this, keys_.cbegin());
  }

  auto end() noexcept -> iterator
  {
    return iterator(*this, keys_.cend());
  }

  auto end() const noexcept -> const_iterator
  {
    return const_iterator(*this, keys_.cend());
  }

  auto cend() const noexcept -> const_iterator
  {
    return const_iterator(*this, keys_.cend());
  }

  friend bool operator==(const path_info&, const path_info&) = default;

private:
  std::string match_pattern_;
  std::string match_path_;
  keys_type keys_;
  map_type map_;
};

}

FITORIA_NAMESPACE_END

#endif
