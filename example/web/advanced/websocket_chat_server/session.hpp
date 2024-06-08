//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "fwd.hpp"

#include <fitoria/web.hpp>

namespace chat {

class session : public std::enable_shared_from_this<session> {
  using context = fitoria::web::websocket::context;

public:
  session(const std::string& room_id,
          const std::string& user_id,
          context& ctx,
          chat_room& room);

  ~session();

  auto run() -> fitoria::awaitable<void>;

  auto async_read() -> fitoria::awaitable<void>;

  auto async_write(std::shared_ptr<std::string> message)
      -> fitoria::awaitable<void>;

  static auto make_session(const std::string& room_id,
                           const std::string& user_id,
                           context& ctx,
                           chat_room& room) -> std::shared_ptr<session>;

private:
  auto async_write_impl() -> fitoria::awaitable<void>;

  std::string room_id_;
  std::string user_id_;
  context& ctx_;
  chat_room& room_;
  boost::asio::experimental::concurrent_channel<
      fitoria::executor_type,
      void(boost::system::error_code, std::shared_ptr<std::string>)>
      channel_;
};

}
