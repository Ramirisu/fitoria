//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_HEADER_ENTITY_TAG_HPP
#define FITORIA_HTTP_HEADER_ENTITY_TAG_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/optional.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http::header {

class entity_tag {
public:
  entity_tag(bool strong, std::string value)
      : strong_(strong)
      , value_(std::move(value))
  {
  }

  auto strong() const noexcept -> bool
  {
    return strong_;
  }

  auto set_strong(bool b) & noexcept -> entity_tag&
  {
    strong_ = b;
    return *this;
  }

  auto set_strong(bool b) && noexcept -> entity_tag&&
  {
    set_strong(b);
    return std::move(*this);
  }

  auto value() const noexcept -> const std::string&
  {
    return value_;
  }

  auto set_value(std::string value) & -> entity_tag&
  {
    value_ = std::move(value);
    return *this;
  }

  auto set_value(std::string value) && -> entity_tag&&
  {
    set_value(std::move(value));
    return std::move(*this);
  }

  auto to_string() const -> std::string
  {
    if (strong_) {
      return fmt::format(R"("{}")", value_);
    }

    return fmt::format(R"(W/"{}")", value_);
  }

  static auto make_strong(std::string value) -> entity_tag
  {
    return { true, std::move(value) };
  }

  static auto make_weak(std::string value) -> entity_tag
  {
    return { false, std::move(value) };
  }

  static auto parse(std::string_view input) -> optional<entity_tag>
  {
    bool strong = true;
    if (input.starts_with("W/")) {
      strong = false;
      input.remove_prefix(2);
    }

    if (input.size() >= 2 && input.starts_with('\"') && input.ends_with('\"')) {
      input.remove_prefix(1);
      input.remove_suffix(1);
      for (auto c : input) {
        if (!is_valid_char(c)) {
          return nullopt;
        }
      }

      return entity_tag(strong, std::string(input));
    }

    return nullopt;
  }

  auto weakly_equal_to(const entity_tag& other) const noexcept -> bool
  {
    return value_ == other.value();
  }

  friend auto operator==(const entity_tag&, const entity_tag&) -> bool
      = default;

private:
  static auto is_valid_char(char c) noexcept -> bool
  {
    return c >= 0x21 && c != '\"';
  }

  bool strong_;
  std::string value_;
};

}

FITORIA_NAMESPACE_END

#endif
