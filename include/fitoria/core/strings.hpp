//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_STRINGS_HPP
#define FITORIA_CORE_STRINGS_HPP

#include <fitoria/core/config.hpp>

#include <algorithm>
#include <cctype>
#include <string_view>
#include <vector>

FITORIA_NAMESPACE_BEGIN

inline auto cmp_eq_ci(std::string_view lhs, std::string_view rhs) -> bool
{
  return std::equal(
      lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](auto l, auto r) {
        return std::tolower(l) == std::tolower(r);
      });
}

inline auto cmp_tw_ci(std::string_view lhs,
                      std::string_view rhs) -> std::strong_ordering
{
  // lexicographical_compare_three_way is available since clang 17
#if defined(FITORIA_TARGET_MACOS)
  auto lfirst = lhs.begin();
  const auto llast = lhs.end();
  auto rfirst = rhs.begin();
  const auto rlast = rhs.end();

  for (; lfirst != llast || rfirst != rlast;) {
    if (lfirst == llast) {
      return rfirst == rlast ? std::strong_ordering::equal
                             : std::strong_ordering::less;
    }
    if (rfirst == rlast) {
      return std::strong_ordering::greater;
    }

    if (auto result = std::tolower(*lfirst) <=> std::tolower(*rfirst);
        result != std::strong_ordering::equal) {
      return result;
    }

    ++lfirst;
    ++rfirst;
  }

  return std::strong_ordering::equal;
#else
  return std::lexicographical_compare_three_way(
      lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](auto l, auto r) {
        return std::tolower(l) <=> std::tolower(r);
      });
#endif
}

inline bool is_space(char c)
{
  return static_cast<bool>(std::isspace(c));
}

template <std::invocable<char> Predicate>
auto ltrim(std::string_view sv, Predicate predicate) -> std::string_view
{
  static_assert(
      std::convertible_to<std::invoke_result_t<Predicate, char>, bool>);

  while (!sv.empty() && static_cast<bool>(predicate(sv.front()))) {
    sv.remove_prefix(1);
  }

  return sv;
}

inline auto ltrim(std::string_view sv) -> std::string_view
{
  return ltrim(sv, is_space);
}

template <std::invocable<char> Predicate>
auto rtrim(std::string_view sv, Predicate predicate) -> std::string_view
{
  static_assert(
      std::convertible_to<std::invoke_result_t<Predicate, char>, bool>);

  while (!sv.empty() && static_cast<bool>(predicate(sv.back()))) {
    sv.remove_suffix(1);
  }

  return sv;
}

inline auto rtrim(std::string_view sv) -> std::string_view
{
  return rtrim(sv, is_space);
}

template <std::invocable<char> Predicate>
auto trim(std::string_view sv, Predicate predicate) -> std::string_view
{
  static_assert(
      std::convertible_to<std::invoke_result_t<Predicate, char>, bool>);

  return ltrim(rtrim(sv, predicate), predicate);
}

inline auto trim(std::string_view sv) -> std::string_view
{
  return ltrim(rtrim(sv));
}

inline auto split_of(std::string_view sv, std::string_view delimiters)
    -> std::vector<std::string_view>
{
  auto tokens = std::vector<std::string_view>();

  auto pos = sv.find_first_of(delimiters);
  while (pos != std::string_view::npos) {
    tokens.push_back(trim(sv.substr(0, pos)));
    sv.remove_prefix(pos + 1);
    pos = sv.find_first_of(delimiters);
  }

  tokens.push_back(trim(sv));

  return tokens;
}

FITORIA_NAMESPACE_END

#endif
