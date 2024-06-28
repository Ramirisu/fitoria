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

FITORIA_NAMESPACE_BEGIN

namespace http::header {

class entity_tag {
public:
  static auto strong(std::string_view tag) -> std::string
  {
    return fmt::format(R"("{}")", tag);
  }

  static auto weak(std::string_view tag) -> std::string
  {
    return fmt::format(R"(W/"{}")", tag);
  }
};

}

FITORIA_NAMESPACE_END

#endif
