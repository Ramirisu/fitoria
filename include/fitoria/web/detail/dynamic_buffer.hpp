//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_DETAIL_DYNAMIC_BUFFER_HPP
#define FITORIA_WEB_DETAIL_DYNAMIC_BUFFER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

#include <utility>

FITORIA_NAMESPACE_BEGIN

namespace web::detail {

template <typename Container>
class dynamic_buffer {
public:
  auto prepare(std::size_t size) -> net::mutable_buffer
  {
    container_.resize(offset_ + size);
    return { container_.data() + offset_, size };
  }

  void commit(std::size_t size)
  {
    offset_ = std::min(offset_ + size, container_.size());
  }

  auto release() -> Container
  {
    container_.resize(offset_);
    return std::move(container_);
  }

private:
  std::size_t offset_ = 0;
  Container container_;
};

}

FITORIA_NAMESPACE_END

#endif
