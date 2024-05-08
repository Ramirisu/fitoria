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

#include <algorithm>
#include <utility>

FITORIA_NAMESPACE_BEGIN

template <typename Container>
class dynamic_buffer {
public:
  using const_buffers_type = net::const_buffer;
  using mutable_buffers_type = net::mutable_buffer;

  auto cdata() const -> const_buffers_type
  {
    return { container_.data() + readable_, size() };
  }

  auto data() const -> const_buffers_type
  {
    return { container_.data() + readable_, size() };
  }

  auto data() -> mutable_buffers_type
  {
    return { container_.data() + readable_, size() };
  }

  void consume(std::size_t size)
  {
    readable_ = std::min(readable_ + size, writable_);
  }

  auto prepare(std::size_t size) -> mutable_buffers_type
  {
    container_.resize(writable_ + size);
    return { container_.data() + writable_, size };
  }

  void commit(std::size_t size)
  {
    writable_ = std::min(writable_ + size, container_.size());
  }

  auto size() const noexcept -> std::size_t
  {
    return writable_ - readable_;
  }

  auto max_size() const noexcept -> std::size_t
  {
    return container_.max_size();
  }

  auto capacity() const noexcept -> std::size_t
  {
    return container_.capacity();
  }

  void compress()
  {
    std::move(container_.begin() + readable_,
              container_.begin() + writable_,
              container_.begin());
    container_.resize(size());
    writable_ = size();
    readable_ = 0;
  }

  auto release() -> Container
  {
    compress();
    return std::move(container_);
  }

private:
  std::size_t readable_ = 0;
  std::size_t writable_ = 0;
  Container container_;
};

FITORIA_NAMESPACE_END

#endif
