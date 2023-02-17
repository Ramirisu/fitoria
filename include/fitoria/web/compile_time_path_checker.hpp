//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

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
  static consteval bool is_valid()
  {
    std::vector<std::string_view> used_param;
    auto it = Pattern.begin();
    while (it != Pattern.end()) {
      if (!on_slash(it, Pattern.end(), used_param)) {
        return false;
      }
    }

    return true;
  }

private:
  static consteval bool
  on_slash(auto& it, const auto end, std::vector<std::string_view>& used_param)
  {
    if (*it != '/') {
      return false;
    }
    if (++it == end) {
      return true;
    }

    return on_param_or_text(it, end, used_param);
  }

  static consteval bool on_param_or_text(
      auto& it, const auto end, std::vector<std::string_view>& used_param)
  {
    if (*it == '{') {
      ++it;
      auto first = it;
      if (it == end || *it == '}') {
        return false;
      }
      while (it != end && *it != '}') {
        if (!on_text(it)) {
          return false;
        }
      }
      if (it == end) {
        return false;
      }
      // *it == '}'
      auto param = std::string_view(first, it);
      if (std::count(used_param.begin(), used_param.end(), param) > 0) {
        return false;
      }
      used_param.push_back(std::string_view(first, it));
      ++it;
    } else {
      while (it != end && *it != '/') {
        if (!on_text(it)) {
          return false;
        }
      }
    }

    return true;
  }

  static consteval bool on_text(auto& it)
  {
    if (*it == '/' || *it == '{' || *it == '}') {
      return false;
    }
    ++it;

    return true;
  }
};
}

FITORIA_NAMESPACE_END

#endif
