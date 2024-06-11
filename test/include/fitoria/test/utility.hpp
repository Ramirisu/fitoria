//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_TEST_UTILITY_HPP
#define FITORIA_TEST_UTILITY_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>

#include <algorithm>
#include <filesystem>
#include <functional>
#include <set>
#include <string_view>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace test {

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

template <typename T>
std::vector<T> str_to_vec(std::string_view str)
{
  auto vec = std::vector<T>();
  vec.reserve(str.size());
  for (auto& c : str) {
    vec.push_back(static_cast<T>(c));
  }

  return vec;
}

inline std::string get_random_temp_file_path()
{
  auto tmp_dir = std::filesystem::temp_directory_path();
  auto file_name = fmt::format(
      "fitoria.test.{}.tmp",
      std::chrono::steady_clock::now().time_since_epoch().count());

  return (tmp_dir / file_name).string();
}

}

FITORIA_NAMESPACE_END

#endif
