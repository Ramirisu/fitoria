//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "session.hpp"

#include "chat_room.hpp"

using namespace fitoria;
using namespace fitoria::web;

namespace chat {

session::session(const std::string& room_id,
                 const std::string& user_id,
                 session::context& ctx,
                 chat_room& room)
    : room_id_(room_id)
    , user_id_(user_id)
    , ctx_(ctx)
    , room_(room)
    , channel_(ctx_.get_executor())
{
  room_.join(room_id_, this);
}

session::~session()
{
  room_.leave(room_id_, this);
}

auto session::run() -> awaitable<void>
{
  using namespace boost::asio::experimental::awaitable_operators;

  // websocket allows one read and one write operation being performed at a
  // time, so we do exactly one read and one write operation here. if any of
  // them stops, cancel the other one and terminate the session.
  co_await (async_read() || async_write_impl());
}

auto session::async_read() -> fitoria::awaitable<void>
{
  for (auto msg = co_await ctx_.async_read(); msg;
       msg = co_await ctx_.async_read()) {
    if (auto text = std::get_if<websocket::text_t>(&*msg); text) {
      co_await room_.broadcast(room_id_,
                               fmt::format("{}: {}", user_id_, text->value));
    } else if (auto c = std::get_if<websocket::close_t>(&*msg); c) {
      break;
    } else {
      co_await ctx_.async_close(websocket::close_code::normal);
      break;
    }
  }
}

auto session::async_write(std::shared_ptr<std::string> message)
    -> awaitable<void>
{
  // websocket only allows one read and one write operation being performed at a
  // time, so we use a concurrent channel as a queue for storing messages.
  [[maybe_unused]] auto result = co_await channel_.async_send(
      boost::system::error_code(), std::move(message), use_awaitable);
}

auto session::make_session(const std::string& room_id,
                           const std::string& user_id,
                           session::context& ctx,
                           chat_room& room) -> std::shared_ptr<session>
{
  return std::make_shared<session>(room_id, user_id, ctx, room);
}

auto session::async_write_impl() -> awaitable<void>
{
  for (;;) {
    // websocket only allows one read and one write operation being performed at
    // a time, so we send messages one by one.
    auto result = co_await channel_.async_receive(use_awaitable);
    if (!result) {
      break;
    }

    co_await ctx_.async_write_text(**result);
  }
}

}
