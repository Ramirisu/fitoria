//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <cassert>

#define FITORIA_NAMESPACE_BEGIN                                                \
  namespace fitoria {                                                          \
  inline namespace v0 {
#define FITORIA_NAMESPACE_END                                                  \
  }                                                                            \
  }

#define FITORIA_ASSERT(expr) assert(expr)
