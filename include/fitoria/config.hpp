//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#define FITORIA_NAMESPACE_BEGIN                                                \
  namespace fitoria {                                                          \
  inline namespace v0 {
#define FITORIA_NAMESPACE_END                                                  \
  }                                                                            \
  }

#include <gul/expected.hpp>
#include <gul/optional.hpp>
#include <gul/string_view.hpp>

FITORIA_NAMESPACE_BEGIN

using gul::in_place;

using gul::expected;
using gul::unexpect;
using gul::unexpected;

using gul::optional;

using gul::string_view;

FITORIA_NAMESPACE_END