//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_DYNAMIC_BUFFER_HPP
#define FITORIA_CORE_DYNAMIC_BUFFER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

#include <iterator>

FITORIA_NAMESPACE_BEGIN

/// @verbatim embed:rst:leading-slashes
///
/// An adapter that turns ``Container`` into a type that satisfies
/// ``net::DynamicBuffer`` concept.
///
/// DESCRIPTION
///   An adapter that turns ``Container`` into a type that satisfies
///   ``net::DynamicBuffer`` concept. Note that ``Container`` must use
///   contiguous storage.
///
///   .. code-block::
///
///                    readable    writable
///      |-----------|-----------|-----------|-----------|-----------|
///      0        roffset     woffset     c.size()  c.capacity()    limit
///
/// @endverbatim
template <typename Container>
class dynamic_buffer {
  static_assert(std::contiguous_iterator<typename Container::iterator>);

public:
  using const_buffers_type = net::const_buffer;
  using mutable_buffers_type = net::mutable_buffer;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct a ``dynamic_buffer`` with maximum capacity of
  /// ``Container::max_size()``.
  ///
  /// @endverbatim
  dynamic_buffer()
      : limit_(container_.max_size())
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct a ``dynamic_buffer`` with maximum capacity of
  /// ``std::size_t limit``.
  ///
  /// @endverbatim
  dynamic_buffer(std::size_t limit)
      : limit_(limit)
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Construct a ``dynamic_buffer`` with existing container.
  ///
  /// @endverbatim
  dynamic_buffer(Container container)
      : container_(std::move(container))
      , roffset_(0)
      , woffset_(container_.size())
      , limit_(container_.max_size())
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get a constant buffer sequence representing readable bytes.
  ///
  /// @endverbatim
  auto cdata() const -> const_buffers_type
  {
    return { container_.data() + roffset_, size() };
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get a constant buffer sequence representing readable bytes.
  ///
  /// @endverbatim
  auto data() const -> const_buffers_type
  {
    return { container_.data() + roffset_, size() };
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get a mutable buffer sequence representing readable bytes.
  ///
  /// @endverbatim
  auto data() -> mutable_buffers_type
  {
    return { container_.data() + roffset_, size() };
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Remove bytes starting from the beginning of readable bytes.
  ///
  /// @endverbatim
  void consume(std::size_t n)
  {
    roffset_ = std::min(roffset_ + n, woffset_);
    if (roffset_ == woffset_) {
      roffset_ = 0;
      woffset_ = 0;
    }
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get a mutable buffer sequence representing writable bytes.
  ///
  /// @endverbatim
  auto prepare(std::size_t n) -> mutable_buffers_type
  {
    if (n <= (limit_ - woffset_)) {
      container_.resize(woffset_ + n);
    } else if (n <= (limit_ - size())) {
      compress();
    } else {
      FITORIA_THROW_OR(std::length_error("dynamic_buffer too large"),
                       std::terminate());
    }

    return { container_.data() + woffset_, n };
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Convert writable bytes into readable bytes.
  ///
  /// @endverbatim
  void commit(std::size_t n)
  {
    woffset_ = std::min(woffset_ + n, container_.size());
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the number of readable bytes.
  ///
  /// @endverbatim
  auto size() const noexcept -> std::size_t
  {
    return woffset_ - roffset_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the maximum allowed of the capacity.
  ///
  /// @endverbatim
  auto max_size() const noexcept -> std::size_t
  {
    return limit_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the capacity.
  ///
  /// @endverbatim
  auto capacity() const noexcept -> std::size_t
  {
    return container_.capacity();
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Release the underlying ``Container`` instance.
  ///
  /// DESCRIPTION
  ///   Release the underlying ``Container`` instance, which only contains
  ///   readable bytes.
  ///
  /// @endverbatim
  auto release() -> Container
  {
    compress();
    container_.resize(size());
    return std::move(container_);
  }

private:
  void compress()
  {
    if (roffset_ > 0) {
      const auto len = size();
      std::memmove(container_.data(), container_.data() + roffset_, len);
      woffset_ = len;
      roffset_ = 0;
    }
  }

  Container container_;
  std::size_t roffset_ = 0;
  std::size_t woffset_ = 0;
  std::size_t limit_;
};

FITORIA_NAMESPACE_END

#endif
