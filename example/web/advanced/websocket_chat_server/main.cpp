//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

#include "chat_room.hpp"
#include "session.hpp"

using namespace fitoria;
using namespace fitoria::web;

struct login_t {
  std::string room_id;
  std::string user_id;
};

auto start_chat(path_of<login_t> login,
                state_of<std::shared_ptr<chat::chat_room>> room,
                websocket ws) -> awaitable<response>
{
  co_return ws.set_handler([login,
                            room](websocket::context& ctx) -> awaitable<void> {
    auto session
        = chat::session::make_session(login.room_id, login.user_id, ctx, *room);
    co_await session->run();
  });
}

// wscat -c http://127.0.0.1:8080/chat/my_room/fitoria

int main()
{
  auto room = std::make_shared<chat::chat_room>();

  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/chat/{room_id}/{user_id}">(start_chat)
                               .use_state(room))
                    .build();
  server.bind("127.0.0.1", 8080);

  ioc.run();
}
