//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_STATE_STORAGE_HPP
#define FITORIA_WEB_STATE_STORAGE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>
#include <fitoria/core/type_traits.hpp>

#include <any>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

using state_map = std::unordered_map<std::type_index, std::any>;
using shared_state_map = std::shared_ptr<state_map>;
using shared_state_maps = std::vector<shared_state_map>;

class state_storage {
public:
  auto copy_append(shared_state_map map) const -> state_storage
  {
    auto copy = *this;
    copy.maps_.push_back(std::move(map));

    return copy;
  }

  auto copy_insert_front(std::type_index ti,
                         std::any obj) const -> state_storage
  {
    auto copy = *this;
    if (copy.maps_.empty()) {
      copy.maps_.push_back(std::make_shared<state_map>());
    }
    (*copy.maps_.front())[ti] = std::move(obj);

    return copy;
  }

  template <typename T>
  auto state() const -> optional<T&>
  {
    static_assert(not_cvref<T>, "T must not be cvref qualified");
    for (auto& map : maps_) {
      if (auto it = map->find(std::type_index(typeid(T))); it != map->end()) {
        return *std::any_cast<T>(&it->second);
      }
    }

    return nullopt;
  }

private:
  shared_state_maps maps_;
};

}

FITORIA_NAMESPACE_END

#endif
