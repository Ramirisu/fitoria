//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "chat_room.hpp"

#include "session.hpp"

namespace chat {

void chat_room::join(session* session)
{
  auto lock = std::scoped_lock(mutex_);
  sessions_.insert(session);
}

void chat_room::leave(session* session)
{
  auto lock = std::scoped_lock(mutex_);
  sessions_.erase(session);
}

auto chat_room::broadcast(std::string message) -> fitoria::awaitable<void>
{
  auto ptr = std::make_shared<std::string>(std::move(message));

  // make the critical section as small as possible by copying the sessions as
  // weak pointers, so we can send messages without holding the mutex later.
  auto sessions = std::vector<std::weak_ptr<session>>();
  auto lock = std::shared_lock(mutex_);
  for (auto& session : sessions_) {
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
