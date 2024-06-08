//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "fwd.hpp"

#include <shared_mutex>
#include <unordered_set>

namespace chat {

class chat_room {
  std::shared_mutex mutex_;
  std::unordered_set<session*> sessions_;

public:
  void join(session* session);

  void leave(session* session);

  auto broadcast(std::string message) -> fitoria::awaitable<void>;
};

}
