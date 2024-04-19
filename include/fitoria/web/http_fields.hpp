//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HTTP_FIELDS_HPP
#define FITORIA_WEB_HTTP_FIELDS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>

#include <fitoria/web/http.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_fields {
private:
  using impl_type = boost::beast::http::fields;

  http_fields(impl_type impl)
      : impl_(std::move(impl))
  {
  }

public:
  using mapped_type = std::string_view;
  using size_type = std::size_t;
  using iterator = typename impl_type::iterator;
  using const_iterator = typename impl_type::const_iterator;

  http_fields() = default;

  http_fields(const http_fields&) = default;

  http_fields(http_fields&&) = default;

  http_fields& operator=(const http_fields&) = default;

  http_fields& operator=(http_fields&&) = default;

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

  auto equal_range(std::string_view name) -> std::pair<iterator, iterator>
  {
    return impl_.equal_range(name);
  }

  auto equal_range(http::field name) -> std::pair<iterator, iterator>
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

  static auto from_impl(impl_type impl) -> http_fields
  {
    return http_fields(std::move(impl));
  }

  void to_impl(impl_type& impl) const
  {
    for (auto& element : impl_) {
      impl.insert(element.name(), element.value());
    }
  }

private:
  impl_type impl_;
};

}

FITORIA_NAMESPACE_END

#endif
