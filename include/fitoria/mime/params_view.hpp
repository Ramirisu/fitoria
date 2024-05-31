//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_MIME_PARAMS_VIEW_HPP
#define FITORIA_MIME_PARAMS_VIEW_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>

#include <unordered_map>

FITORIA_NAMESPACE_BEGIN

namespace mime {

class params_view {
  using impl_type = std::unordered_map<std::string_view, std::string_view>;

public:
  using key_type = typename impl_type::key_type;
  using mapped_type = typename impl_type::mapped_type;
  using value_type = typename impl_type::value_type;
  using size_type = typename impl_type::size_type;
  using difference_type = typename impl_type::difference_type;
  using reference = typename impl_type::reference;
  using const_reference = typename impl_type::const_reference;
  using pointer = typename impl_type::pointer;
  using const_pointer = typename impl_type::const_pointer;
  using iterator = typename impl_type::iterator;
  using const_iterator = typename impl_type::const_iterator;

  params_view() = default;

  params_view(std::initializer_list<value_type> init)
      : impl_(init.begin(), init.end())
  {
  }

  params_view(const params_view&) = default;

  params_view(params_view&&) = default;

  params_view& operator=(const params_view&) = default;

  params_view& operator=(params_view&&) = default;

  auto empty() const noexcept -> bool
  {
    return impl_.empty();
  }

  auto size() const noexcept -> size_type
  {
    return impl_.size();
  }

  auto max_size() const noexcept -> size_type
  {
    return impl_.max_size();
  }

  auto contains(std::string_view name) const noexcept -> bool
  {
    return impl_.contains(name);
  }

  void clear() noexcept
  {
    impl_.clear();
  }

  void set(std::string_view name, std::string_view value)
  {
    impl_[name] = value;
  }

  auto get(std::string_view name) noexcept -> optional<mapped_type>
  {
    if (auto it = impl_.find(name); it != impl_.end()) {
      return { it->second };
    }

    return nullopt;
  }

  auto get(std::string_view name) const noexcept -> optional<mapped_type>
  {
    if (auto it = impl_.find(name); it != impl_.end()) {
      return { it->second };
    }

    return nullopt;
  }

  auto erase(std::string_view name) -> optional<mapped_type>
  {
    if (auto it = impl_.find(name); it != impl_.end()) {
      auto value = it->second;
      impl_.erase(it);
      return value;
    }

    return nullopt;
  }

  auto at(std::string_view name) -> mapped_type&
  {
    return impl_.at(name);
  }

  auto at(std::string_view name) const -> const mapped_type&
  {
    return impl_.at(name);
  }

  auto operator[](std::string_view name) -> mapped_type&
  {
    return impl_[name];
  }

  auto begin() noexcept -> iterator
  {
    return impl_.begin();
  }

  auto begin() const noexcept -> const_iterator
  {
    return impl_.begin();
  }

  auto cbegin() const noexcept -> const_iterator
  {
    return impl_.cbegin();
  }

  auto end() noexcept -> iterator
  {
    return impl_.end();
  }

  auto end() const noexcept -> const_iterator
  {
    return impl_.end();
  }

  auto cend() const noexcept -> const_iterator
  {
    return impl_.cend();
  }

  friend bool operator==(const params_view&, const params_view&) = default;

private:
  impl_type impl_;
};

}

FITORIA_NAMESPACE_END

#endif
