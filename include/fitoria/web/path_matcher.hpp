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

#include <boost/regex.hpp>

#include <string>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

class path_matcher {
public:
  enum class token_kind {
    static_,
    named_param,
    wildcard,
  };

  struct token_t {
    token_kind kind;
    std::string value;

    friend bool operator==(const token_t&, const token_t&) = default;
  };

  using tokens_t = std::vector<token_t>;

  path_matcher(std::string_view pattern)
      : pattern_(std::string(pattern))
      , tokens_(parse_pattern(pattern))
      , regex_(to_regex(tokens_))
  {
  }

  auto pattern() const noexcept -> std::string_view
  {
    return pattern_;
  }

  auto tokens() const noexcept -> const tokens_t&
  {
    return tokens_;
  }

  auto match(const std::string& input) const
      -> optional<std::vector<std::pair<std::string, std::string>>>
  {
    if (boost::smatch match; boost::regex_match(input, match, regex_)) {
      std::vector<std::pair<std::string, std::string>> matches;
      for (auto token : tokens_) {
        if (token.kind == token_kind::named_param
            || token.kind == token_kind::wildcard) {
          matches.push_back({ token.value, match[token.value].str() });
        }
      }

      return matches;
    }

    return nullopt;
  }

  static tokens_t parse_pattern(std::string_view pattern)
  {
    tokens_t tokens;
    std::string::size_type prev = 0;
    for (auto curr = pattern.find('{'); curr != std::string_view::npos;
         curr = pattern.find('{', curr)) {

      auto next = pattern.find('}', curr + 1);
      tokens.push_back(
          token_t { .kind = token_kind::static_,
                    .value = std::string(pattern.substr(prev, curr - prev)) });
      tokens.push_back(token_t {
          .kind = token_kind::named_param,
          .value = std::string(pattern.substr(curr + 1, next - curr - 1)) });

      prev = next + 1;
      curr = prev;
    }

    if (auto curr = pattern.find('#', prev); curr != std::string_view::npos) {
      if (curr - prev > 0) {
        tokens.push_back(token_t {
            .kind = token_kind::static_,
            .value = std::string(pattern.substr(prev, curr - prev)) });
      }
      tokens.push_back(
          token_t { .kind = token_kind::wildcard,
                    .value = std::string(pattern.substr(curr + 1)) });
    } else {
      if (auto suffix = pattern.substr(prev); !suffix.empty()) {
        tokens.push_back(token_t { .kind = token_kind::static_,
                                   .value = std::string(suffix) });
      }
    }

    return tokens;
  }

private:
  static boost::regex to_regex(const tokens_t& tokens)
  {
    std::string regex;
    for (auto& token : tokens) {
      if (token.kind == token_kind::static_) {
        regex += token.value;
      } else if (token.kind == token_kind::named_param) {
        // {name} => (?<name>pattern)
        regex += "(?<";
        regex += token.value;
        regex += ">[^{}/]+)";
      } else if (token.kind == token_kind::wildcard) {
        // {name} => (?<name>pattern)
        regex += "(?<";
        regex += token.value;
        regex += ">.+)";
      }
    }

    return boost::regex(regex);
  }

  std::string pattern_;
  tokens_t tokens_;
  boost::regex regex_;
};

}

FITORIA_NAMESPACE_END

#endif
