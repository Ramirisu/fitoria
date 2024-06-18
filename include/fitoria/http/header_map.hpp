//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_HEADER_MAP_HPP
#define FITORIA_HTTP_HEADER_MAP_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>

#include <fitoria/http/field.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http {

/// @verbatim embed:rst:leading-slashes
///
/// Provides access to the HTTP headers.
///
/// @endverbatim
class header_map {
private:
  using impl_type = boost::beast::http::fields;

  header_map(impl_type impl)
      : impl_(std::move(impl))
  {
  }

public:
  using mapped_type = std::string_view;
  using size_type = std::size_t;
  using iterator = typename impl_type::iterator;
  using const_iterator = typename impl_type::const_iterator;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct an empty header map.
  ///
  /// @endverbatim
  header_map() = default;

  header_map(const header_map&) = default;

  header_map(header_map&&) = default;

  header_map& operator=(const header_map&) = default;

  header_map& operator=(header_map&&) = default;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Check whether the container is empty.
  ///
  /// @endverbatim
  auto empty() const noexcept -> bool
  {
    return begin() == end();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the number of key/value pairs.
  ///
  /// @endverbatim
  auto size() const noexcept -> size_type
  {
    return std::distance(begin(), end());
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Checks whether the container contains element with specific key.
  ///
  /// DESCRIPTION
  ///   Checks whether the container contains element with specific key. The
  ///   input ``name`` will be canonicalized before searching.
  ///
  /// @endverbatim
  auto contains(std::string_view name) const -> bool
  {
    return impl_.find(name) != end();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Checks whether the container contains element with specific key.
  ///
  /// DESCRIPTION
  ///   Checks whether the container contains element with specific key.
  ///
  /// @endverbatim
  auto contains(http::field name) const -> bool
  {
    return impl_.find(name) != end();
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
  /// Set HTTP headers.
  ///
  /// DESCRIPTION
  ///   Set HTTP headers. The input ``name`` will be canonicalized before
  ///   insertion. Note that any existing headers with the same name will be
  ///   removed before the insertion.
  ///
  /// @endverbatim
  void set(std::string_view name, std::string_view value)
  {
    impl_.set(name, value);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set HTTP headers.
  ///
  /// DESCRIPTION
  ///   Set HTTP headers. Note that any existing headers with the same name
  ///   will be removed before the insertion.
  ///
  /// @endverbatim
  void set(http::field name, std::string_view value)
  {
    impl_.set(name, value);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Insert HTTP headers.
  ///
  /// DESCRIPTION
  ///   Insert HTTP headers. The input ``name`` will be canonicalized before
  ///   insertion. Note that any existing headers with the same name will be
  ///   kept.
  ///
  /// @endverbatim
  void insert(std::string_view name, std::string_view value)
  {
    impl_.insert(name, value);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Insert HTTP headers.
  ///
  /// DESCRIPTION
  ///   Insert HTTP headers. Note that any existing headers with the same name
  ///   will be kept.
  ///
  /// @endverbatim
  void insert(http::field name, std::string_view value)
  {
    impl_.insert(name, value);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get header with specific key.
  ///
  /// DESCRIPTION
  ///   Get header with specific key. The input ``name`` will be canonicalized
  ///   before searching. Note that if there are multiple headers with the same
  ///   name, only the first one will be returned. Consider using
  ///   ``equal_range`` instead for iterating all the headers with the same
  ///   name.
  ///
  /// @endverbatim
  auto get(std::string_view name) const noexcept -> optional<mapped_type>
  {
    if (auto it = impl_.find(name); it != impl_.end()) {
      return it->value();
    }

    return nullopt;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get header with specific key.
  ///
  /// DESCRIPTION
  ///   Get header with specific key. Note that if there are multiple headers
  ///   with the same name, only the first one will be returned. Consider using
  ///   ``equal_range`` instead for iterating all the headers with the same
  ///   name.
  ///
  /// @endverbatim
  auto get(http::field name) const noexcept -> optional<mapped_type>
  {
    if (auto it = impl_.find(name); it != impl_.end()) {
      return it->value();
    }

    return nullopt;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get range of elements matching a specific key.
  ///
  /// DESCRIPTION
  ///   Get range of elements matching a specific key. The input ``name`` will
  ///   be canonicalized before searching.
  ///
  /// @endverbatim
  auto equal_range(std::string_view name) const -> std::pair<iterator, iterator>
  {
    return impl_.equal_range(name);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get range of elements matching a specific key.
  ///
  /// DESCRIPTION
  ///   Get range of elements matching a specific key.
  ///
  /// @endverbatim
  auto equal_range(http::field name) const -> std::pair<iterator, iterator>
  {
    return impl_.equal_range(name);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Remove element with specific key.
  ///
  /// DESCRIPTION
  ///   Remove element with specific key. The input ``name`` will be
  ///   canonicalized before searching.
  ///
  /// @endverbatim
  auto erase(std::string_view name) -> size_type
  {
    return impl_.erase(name);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Remove element with specific key.
  ///
  /// DESCRIPTION
  ///   Remove element with specific key.
  ///
  /// @endverbatim
  auto erase(http::field name) -> size_type
  {
    return impl_.erase(name);
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

  static auto from_impl(impl_type impl) -> header_map
  {
    return { std::move(impl) };
  }

  void to_impl(impl_type& impl) const
  {
    for (auto& element : impl_) {
      impl.insert(element.name_string(), element.value());
    }
  }

private:
  impl_type impl_;
};

}

FITORIA_NAMESPACE_END

#endif
