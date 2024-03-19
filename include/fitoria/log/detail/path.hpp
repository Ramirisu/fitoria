//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_DETAIL_PATH_HPP
#define FITORIA_LOG_DETAIL_PATH_HPP

#include <fitoria/core/config.hpp>

#include <string_view>

FITORIA_NAMESPACE_BEGIN

namespace log::detail {

inline std::string_view get_file_name(std::string_view file_path) noexcept
{
  if (auto pos = file_path.find_last_of("\\/"); pos != std::string_view::npos) {
    return file_path.substr(pos + 1);
  }

  return file_path;
}

}

FITORIA_NAMESPACE_END

#endif
