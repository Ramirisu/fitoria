//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_DETAIL_UNORDERED_STRING_MAP_HPP
#define FITORIA_CORE_DETAIL_UNORDERED_STRING_MAP_HPP

#include <fitoria/core/config.hpp>

#include <boost/core/detail/string_view.hpp>

#include <string>
#include <string_view>

FITORIA_NAMESPACE_BEGIN

namespace detail {

struct string_hash {
  using is_transparent = void;

  size_t operator()(const char* s) const
  {
    return std::hash<std::string_view> {}(s);
  }

  size_t operator()(std::string_view sv) const
  {
    return std::hash<std::string_view> {}(sv);
  }

  size_t operator()(const std::string& s) const
  {
    return std::hash<std::string> {}(s);
  }

  size_t operator()(boost::core::string_view sv) const
  {
    return std::hash<std::string_view> {}(
        std::string_view(sv.data(), sv.size()));
  }
};

}

FITORIA_NAMESPACE_END

#endif
