//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_DETAIL_STRING_HPP
#define FITORIA_WEB_DETAIL_STRING_HPP

#include <fitoria/core/config.hpp>

#include <cctype>
#include <string_view>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web::detail {

inline auto ltrim(std::string_view s) noexcept -> std::string_view
{
  while (!s.empty() && std::isspace(s.front())) {
    s.remove_prefix(1);
  }

  return s;
}

inline auto rtrim(std::string_view s) noexcept -> std::string_view
{
  while (!s.empty() && std::isspace(s.back())) {
    s.remove_suffix(1);
  }

  return s;
}

inline auto trim(std::string_view s) noexcept -> std::string_view
{
  return ltrim(rtrim(s));
}

inline auto split_of(std::string_view sv, std::string_view delim)
    -> std::vector<std::string_view>
{
  auto tokens = std::vector<std::string_view>();

  auto pos = sv.find_first_of(delim);
  while (pos != std::string_view::npos) {
    tokens.push_back(trim(sv.substr(0, pos)));
    sv.remove_prefix(pos + 1);
    pos = sv.find_first_of(delim);
  }

  tokens.push_back(trim(sv));

  return tokens;
}

}

FITORIA_NAMESPACE_END

#endif
