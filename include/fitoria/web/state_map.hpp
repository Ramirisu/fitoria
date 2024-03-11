//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_STATE_MAP_HPP
#define FITORIA_WEB_STATE_MAP_HPP

#include <fitoria/core/config.hpp>

#include <any>
#include <memory>
#include <typeindex>
#include <unordered_map>

FITORIA_NAMESPACE_BEGIN

namespace web {

using state_map = std::unordered_map<std::type_index, std::any>;
using shared_state_map = std::shared_ptr<state_map>;

}

FITORIA_NAMESPACE_END

#endif
