//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "chat_room.hpp"

#include "session.hpp"

namespace chat {

void chat_room::join(room_id room_id, session* session)
{
  auto lock = std::scoped_lock(mutex_);
  rooms_[room_id].insert(session);
}

void chat_room::leave(room_id room_id, session* session)
{
  auto lock = std::scoped_lock(mutex_);
  if (auto it = rooms_.find(room_id); it != rooms_.end()) {
    it->second.erase(session);

    // if all the sessions are removed, close the room.
    if (it->second.empty()) {
      rooms_.erase(it);
    }
  }
}

auto chat_room::broadcast(room_id room_id, std::string message)
    -> fitoria::awaitable<void>
{
  auto ptr = std::make_shared<std::string>(std::move(message));

  // make the critical section as small as possible by copying the sessions as
  // weak pointers, so we can send messages without holding the mutex later.
  auto sessions = std::vector<std::weak_ptr<session>>();
  auto lock = std::shared_lock(mutex_);
  for (auto& session : rooms_[room_id]) {
    sessions.emplace_back(session->weak_from_this());
  }
  lock.unlock();

  for (auto& session : sessions) {
    if (auto sp = session.lock(); sp) {
      co_await sp->async_write(ptr);
    }
  }
}

}
