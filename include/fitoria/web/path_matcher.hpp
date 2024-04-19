//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_PATH_MATCHER_HPP
#define FITORIA_WEB_PATH_MATCHER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/fixed_string.hpp>
#include <fitoria/core/optional.hpp>

#include <fitoria/web/error.hpp>
#include <fitoria/web/path_parser.hpp>

#include <boost/regex.hpp>

#include <string>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

class path_matcher {
public:
  path_matcher(std::string_view pattern)
      : pattern_(std::string(pattern))
  {
    auto parser = path_parser<true>();
    if (!parser.parse(pattern)) {
      // the path should already be parsed at compile-time...
      // TODO: throw?
      std::terminate();
    }
    tokens_ = parser.get();
    regex_ = to_regex(tokens_);
  }

  auto pattern() const noexcept -> std::string_view
  {
    return pattern_;
  }

  auto tokens() const noexcept -> const path_tokens_t&
  {
    return tokens_;
  }

  auto match(const std::string& input) const
      -> optional<std::vector<std::pair<std::string, std::string>>>
  {
    if (boost::smatch match; boost::regex_match(input, match, regex_)) {
      std::vector<std::pair<std::string, std::string>> matches;
      for (auto token : tokens_) {
        if (token.kind == path_token_kind::param
            || token.kind == path_token_kind::wildcard) {
          matches.push_back({ token.value, match[token.value].str() });
        }
      }

      return matches;
    }

    return nullopt;
  }

private:
  static auto to_regex(const path_tokens_t& tokens) -> boost::regex
  {
    std::string regex;
    for (auto& token : tokens) {
      if (token.kind == path_token_kind::static_) {
        regex += token.value;
      } else if (token.kind == path_token_kind::param) {
        // {name} => (?<name>pattern)
        regex += "(?<";
        regex += token.value;
        regex += ">[^{}/]+)";
      } else if (token.kind == path_token_kind::wildcard) {
        // {name} => (?<name>pattern)
        regex += "(?<";
        regex += token.value;
        regex += ">.*)";
      }
    }

    return boost::regex(regex);
  }

  std::string pattern_;
  path_tokens_t tokens_;
  boost::regex regex_;
};

}

FITORIA_NAMESPACE_END

#endif
