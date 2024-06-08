//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "fwd.hpp"

#include <fitoria/web.hpp>

#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

namespace chat {

class chat_room {
  std::shared_mutex mutex_;
  std::unordered_map<room_id, std::unordered_set<session*>> rooms_;

public:
  void join(room_id room_id, session* session);

  void leave(room_id room_id, session* session);

  auto broadcast(room_id room_id,
                 std::string message) -> fitoria::awaitable<void>;
};

}
