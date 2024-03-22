//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_PATH_PARSER_HPP
#define FITORIA_WEB_PATH_PARSER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/fixed_string.hpp>

#include <string>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

enum class path_token_kind {
  static_,
  param,
  wildcard,
};

struct path_token_t {
  path_token_kind kind;
  std::string value;

  friend bool operator==(const path_token_t&, const path_token_t&) = default;
};

using path_tokens_t = std::vector<path_token_t>;

template <bool Wildcard>
class path_parser {
public:
  constexpr path_parser() = default;

  constexpr bool parse(std::string_view pattern)
  {
    init(pattern);
    return on_root();
  }

  template <basic_fixed_string Pattern>
  constexpr bool parse()
  {
    init(Pattern);
    return on_root();
  }

  constexpr auto get() -> path_tokens_t
  {
    return std::move(tokens_);
  }

private:
  constexpr void init(std::string_view pattern)
  {
    token_first_ = pattern.begin();
    prev_ = pattern.begin();
    it_ = pattern.begin();
    last_ = pattern.end();
    kind_ = path_token_kind::static_;
    tokens_.clear();
  }

  constexpr bool on_root()
  {
    while (it_ != last_) {
      if (kind_ == path_token_kind::static_) {
        if (*it_ == '{') {
          auto value = std::string(token_first_, it_);
          if (value.empty() || !try_push_without_duplicate(kind_, value)) {
            return false;
          }
          kind_ = path_token_kind::param;
          if (prev_ == it_ || *prev_ != '/') {
            return false;
          }
          advance_it();
          token_first_ = it_;
        } else if (is_wildcard(*it_)) {
          if (!Wildcard) {
            return false;
          }
          auto value = std::string(token_first_, it_);
          if (value.empty() || !try_push_without_duplicate(kind_, value)) {
            return false;
          }
          kind_ = path_token_kind::wildcard;
          advance_it();
          token_first_ = it_;
        } else {
          if (*it_ == '/') {
            advance_it();
          } else if (!on_pchar()) {
            return false;
          }
        }
      } else if (kind_ == path_token_kind::param) {
        if (*it_ == '}') {
          auto value = std::string(token_first_, it_);
          if (value.empty() || !try_push_without_duplicate(kind_, value)) {
            return false;
          }
          kind_ = path_token_kind::static_;
          advance_it();
          token_first_ = it_;
          if (it_ != last_ && *it_ != '/') {
            return false;
          }
        } else if (*it_ == '/' || is_wildcard(*it_) || !on_pchar()) {
          return false;
        }
      } else { // kind_ == path_token_kind::wildcard
        if (!on_pchar()) {
          return false;
        }
      }
    }

    if (kind_ == path_token_kind::static_) {
      if (auto value = std::string(token_first_, it_); !value.empty()) {
        if (!try_push_without_duplicate(kind_, value)) {
          return false;
        }
      }
    } else if (kind_ == path_token_kind::param) {
      return false;
    } else { // kind_ == path_token_kind::wildcard
      if (auto value = std::string(token_first_, it_);
          value.empty() || !try_push_without_duplicate(kind_, value)) {
        return false;
      }
    }

    return true;
  }

  constexpr bool try_push_without_duplicate(path_token_kind kind,
                                            std::string value)
  {
    for (auto& token : tokens_) {
      if (kind != path_token_kind::static_
          && token.kind != path_token_kind::static_ && value == token.value) {
        return false;
      }
    }

    tokens_.push_back({ kind, value });

    return true;
  }

  constexpr void advance_it()
  {
    prev_ = it_++;
  }

  static constexpr bool is_wildcard(char c)
  {
    return c == '#';
  }

  constexpr bool on_pchar()
  {
    if (is_unreserved(*it_) || is_sub_delims(*it_) || *it_ == ':'
        || *it_ == '@') {
      advance_it();
      return true;
    }

    if (*it_ == '%' && on_pct_encoded()) {
      return true;
    }

    return false;
  }

  constexpr bool on_pct_encoded()
  {
    advance_it(); // '%'
    if (it_ == last_ || !is_hex(*it_)) {
      return false;
    }
    advance_it();
    if (it_ == last_ || !is_hex(*it_)) {
      return false;
    }
    advance_it();
    return true;
  }

  static constexpr bool is_unreserved(char c)
  {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_'
        || c == '~';
  }

  static constexpr bool is_sub_delims(char c)
  {
    return c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')'
        || c == '*' || c == '+' || c == ',' || c == ';' || c == '=';
  }

  static constexpr bool is_hex(char c)
  {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
        || (c >= 'A' && c <= 'F');
  }

  std::string_view::const_iterator token_first_;
  std::string_view::const_iterator prev_;
  std::string_view::const_iterator it_;
  std::string_view::const_iterator last_;
  path_token_kind kind_ = path_token_kind::static_;
  path_tokens_t tokens_;
};

}

FITORIA_NAMESPACE_END

#endif
