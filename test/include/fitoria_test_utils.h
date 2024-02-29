//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_FITORIA_TEST_UTILS_HPP
#define FITORIA_FITORIA_TEST_UTILS_HPP

#include <fitoria/core/expected.hpp>
#include <fitoria/core/lazy.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/optional.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <set>
#include <span>
#include <type_traits>
#include <vector>

template <typename L, typename R, typename Comparator = std::equal_to<>>
bool range_equal(const L& lhs,
                 const R& rhs,
                 Comparator comparator = Comparator())
{
  return std::equal(
      lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::move(comparator));
}

template <typename Iter, typename Projection, typename T>
bool range_in_set(std::pair<Iter, Iter> iters,
                  Projection projection,
                  std::set<T> set)
{
  while (iters.first != iters.second) {
    auto&& value = std::invoke(projection, iters.first);
    if (set.contains(value)) {
      set.erase(value);
    }
    ++iters.first;
  }

  return set.empty();
}

std::vector<std::byte> to_bytes(std::string_view str)
{
  auto s = std::as_bytes(std::span(str.begin(), str.end()));
  return { s.begin(), s.end() };
}

namespace fitoria {

template <std::size_t ChunkSize>
class test_async_readable_chunk_stream {
public:
  test_async_readable_chunk_stream(std::string_view sv)
      : data_(std::as_bytes(std::span(sv.data(), sv.size())))
  {
  }

  template <typename T, std::size_t N>
  test_async_readable_chunk_stream(std::span<T, N> s)
      : data_(std::as_bytes(s))
  {
  }

  auto is_chunked() const noexcept
  {
    return true;
  }

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    return data_.size();
  }

  auto async_read_next()
      -> lazy<optional<expected<std::vector<std::byte>, net::error_code>>>
  {
    if (written >= data_.size()) {
      co_return nullopt;
    }

    const auto chunk_size = std::min(ChunkSize, data_.size() - written);
    auto sub = data_.subspan(written, chunk_size);
    written += chunk_size;
    co_return std::vector<std::byte>(sub.begin(), sub.end());
  }

private:
  std::span<const std::byte> data_;
  std::size_t written = 0;
};

}

#endif
