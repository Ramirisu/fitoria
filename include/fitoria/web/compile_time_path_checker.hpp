//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_COMPILE_TIME_PATH_CHECKER_HPP
#define FITORIA_WEB_COMPILE_TIME_PATH_CHECKER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/fixed_string.hpp>

#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

class compile_time_path_checker {
public:
  template <basic_fixed_string Pattern>
  static consteval bool is_valid_scope()
  {
    return on_path(Pattern.begin(), Pattern.end(), false);
  }

  template <basic_fixed_string Pattern>
  static consteval bool is_valid_route()
  {
    return on_path(Pattern.begin(), Pattern.end(), true);
  }

private:
  static consteval bool
  on_path(auto&& it, const auto last, bool trailing_wildcard)
  {
    std::vector<std::string_view> used_params;
    while (it != last) {
      if (*it != '/' || !on_segment(it, last, trailing_wildcard, used_params)) {
        return false;
      }
    }

    return true;
  }

  static consteval bool on_segment(auto& it,
                                   const auto last,
                                   bool trailing_wildcard,
                                   std::vector<std::string_view>& used_params)
  {
    ++it; // '/'
    if (it == last) {
      return true;
    }

    return on_segment_content(it, last, trailing_wildcard, used_params);
  }

  static consteval bool
  on_segment_content(auto& it,
                     const auto last,
                     bool trailing_wildcard,
                     std::vector<std::string_view>& used_params)
  {
    if (*it == '{') {
      return on_param(it, last, used_params);
    }

    while (it != last && *it != '/') {
      if (is_wildcard(*it)) {
        return trailing_wildcard && on_wildcard(it, last, used_params);
      }
      if (!on_pchar(it, last)) {
        return false;
      }
    }

    return true;
  }

  static consteval bool on_param(auto& it,
                                 const auto last,
                                 std::vector<std::string_view>& used_params)
  {
    ++it; // '{'
    auto first = it;
    while (it != last && *it != '}') {
      if (is_wildcard(*it) || !on_pchar(it, last)) {
        return false;
      }
    }

    if (it == last || std::distance(first, it) == 0) {
      return false;
    }

    auto param = std::string_view(first, it);
    ++it; // '}'

    return try_push(used_params, param);
  }

  static consteval bool on_wildcard(auto& it,
                                    const auto last,
                                    std::vector<std::string_view>& used_params)
  {
    ++it; // '#'
    auto first = it;
    while (it != last) {
      if (!on_pchar(it, last)) {
        return false;
      }
    }

    return try_push(used_params, std::string_view(first, it));
  }

  static consteval bool is_wildcard(char c)
  {
    return c == '#';
  }

  static consteval bool on_pchar(auto& it, const auto last)
  {
    if (is_unreserved(*it) || is_sub_delims(*it) || *it == ':' || *it == '@') {
      ++it;
      return true;
    }

    if (*it == '%' && on_pct_encoded(it, last)) {
      return true;
    }

    return false;
  }

  static consteval bool on_pct_encoded(auto& it, const auto last)
  {
    ++it; // '%'
    if (it == last || !is_hex(*it)) {
      return false;
    }
    ++it;
    if (it == last || !is_hex(*it)) {
      return false;
    }
    ++it;
    return true;
  }

  static consteval bool is_unreserved(char c)
  {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_'
        || c == '~';
  }

  static consteval bool is_sub_delims(char c)
  {
    return c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')'
        || c == '*' || c == '+' || c == ',' || c == ';' || c == '=';
  }

  static consteval bool is_hex(char c)
  {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
        || (c >= 'A' && c <= 'F');
  }

  static consteval bool try_push(std::vector<std::string_view>& used_params,
                                 std::string_view param)
  {
    if (std::any_of(used_params.begin(),
                    used_params.end(),
                    [param](auto& used) { return used == param; })) {
      return false;
    }

    used_params.push_back(param);

    return true;
  }
};
}

FITORIA_NAMESPACE_END

#endif
