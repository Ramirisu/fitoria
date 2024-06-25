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

#include <fitoria/mime/name_view.hpp>

#include <map>

FITORIA_NAMESPACE_BEGIN

namespace mime {

/// @verbatim embed:rst:leading-slashes
///
/// A type for dealing with mime parameters.
///
/// @endverbatim
class params_view {
  using impl_type = std::map<name_view, name_view>;

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

  /// @verbatim embed:rst:leading-slashes
  ///
  /// The default constructor creates an empty view.
  ///
  /// @endverbatim
  params_view() = default;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct the view by a list of key/value pairs.
  ///
  /// @endverbatim
  params_view(std::initializer_list<value_type> init)
      : impl_(init.begin(), init.end())
  {
  }

  params_view(const params_view&) = default;

  params_view(params_view&&) = default;

  params_view& operator=(const params_view&) = default;

  params_view& operator=(params_view&&) = default;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Check whether the container is empty.
  ///
  /// @endverbatim
  auto empty() const noexcept -> bool
  {
    return impl_.empty();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the number of key/value pairs.
  ///
  /// @endverbatim
  auto size() const noexcept -> size_type
  {
    return impl_.size();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the maximum possible number of key/value pairs.
  ///
  /// @endverbatim
  auto max_size() const noexcept -> size_type
  {
    return impl_.max_size();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Checks whether the container contains element with specific key.
  ///
  /// @endverbatim
  auto contains(name_view name) const noexcept -> bool
  {
    return impl_.contains(name);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Clear the contents.
  ///
  /// @endverbatim
  void clear() noexcept
  {
    impl_.clear();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Insert new element or update value of existing element with specific key.
  ///
  /// @endverbatim
  void set(name_view name, name_view value)
  {
    impl_[name] = value;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get value with specific key.
  ///
  /// @endverbatim
  auto get(name_view name) noexcept -> optional<mapped_type>
  {
    if (auto it = impl_.find(name); it != impl_.end()) {
      return { it->second };
    }

    return nullopt;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get value with specific key.
  ///
  /// @endverbatim
  auto get(name_view name) const noexcept -> optional<mapped_type>
  {
    if (auto it = impl_.find(name); it != impl_.end()) {
      return { it->second };
    }

    return nullopt;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Remove element with specific key.
  ///
  /// @endverbatim
  auto erase(name_view name) -> optional<mapped_type>
  {
    if (auto it = impl_.find(name); it != impl_.end()) {
      auto value = it->second;
      impl_.erase(it);
      return value;
    }

    return nullopt;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get value with specific key.
  ///
  /// @endverbatim
  auto at(name_view name) -> mapped_type&
  {
    return impl_.at(name);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get value with specific key.
  ///
  /// @endverbatim
  auto at(name_view name) const -> const mapped_type&
  {
    return impl_.at(name);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get or insert specific element.
  ///
  /// @endverbatim
  auto operator[](name_view name) -> mapped_type&
  {
    return impl_[name];
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning.
  ///
  /// @endverbatim
  auto begin() noexcept -> iterator
  {
    return impl_.begin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning.
  ///
  /// @endverbatim
  auto begin() const noexcept -> const_iterator
  {
    return impl_.begin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the beginning.
  ///
  /// @endverbatim
  auto cbegin() const noexcept -> const_iterator
  {
    return impl_.cbegin();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end.
  ///
  /// @endverbatim
  auto end() noexcept -> iterator
  {
    return impl_.end();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end.
  ///
  /// @endverbatim
  auto end() const noexcept -> const_iterator
  {
    return impl_.end();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get an iterator to the end.
  ///
  /// @endverbatim
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
