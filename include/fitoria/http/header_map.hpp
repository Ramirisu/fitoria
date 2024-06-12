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

  header_map() = default;

  header_map(const header_map&) = default;

  header_map(header_map&&) = default;

  header_map& operator=(const header_map&) = default;

  header_map& operator=(header_map&&) = default;

  auto empty() const noexcept -> bool
  {
    return begin() == end();
  }

  auto size() const noexcept -> size_type
  {
    return std::distance(begin(), end());
  }

  void clear() noexcept
  {
    impl_.clear();
  }

  void set(std::string_view name, std::string_view value)
  {
    impl_.set(name, value);
  }

  void set(http::field name, std::string_view value)
  {
    impl_.set(name, value);
  }

  void insert(std::string_view name, std::string_view value)
  {
    impl_.insert(name, value);
  }

  void insert(http::field name, std::string_view value)
  {
    impl_.insert(name, value);
  }

  auto get(std::string_view name) const noexcept -> optional<mapped_type>
  {
    if (auto it = impl_.find(name); it != impl_.end()) {
      return it->value();
    }

    return nullopt;
  }

  auto get(http::field name) const noexcept -> optional<mapped_type>
  {
    if (auto it = impl_.find(name); it != impl_.end()) {
      return it->value();
    }

    return nullopt;
  }

  auto equal_range(std::string_view name) const -> std::pair<iterator, iterator>
  {
    return impl_.equal_range(name);
  }

  auto equal_range(http::field name) const -> std::pair<iterator, iterator>
  {
    return impl_.equal_range(name);
  }

  auto erase(std::string_view name) -> size_type
  {
    return impl_.erase(name);
  }

  auto erase(http::field name) -> size_type
  {
    return impl_.erase(name);
  }

  auto contains(std::string_view name) const -> bool
  {
    return impl_.find(name) != end();
  }

  auto contains(http::field name) const -> bool
  {
    return impl_.find(name) != end();
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
