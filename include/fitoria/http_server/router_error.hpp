//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

FITORIA_NAMESPACE_BEGIN

enum class router_error {
  parse_path_error,
  route_already_exists,
  route_not_exists,
};

FITORIA_NAMESPACE_END
