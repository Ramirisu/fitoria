//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_DETAIL_BROTLI_PARAMS_HPP
#define FITORIA_WEB_DETAIL_BROTLI_PARAMS_HPP

#include <fitoria/core/config.hpp>

#include <cstddef>
#include <cstdint>

FITORIA_NAMESPACE_BEGIN

namespace web::detail {

struct broti_params {
  const std::uint8_t* next_in;
  std::size_t avail_in = 0;
  std::uint8_t* next_out;
  std::size_t avail_out = 0;
};

}

FITORIA_NAMESPACE_END

#endif
