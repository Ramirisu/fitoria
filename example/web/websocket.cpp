//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

auto echo(websocket::context& ctx) -> awaitable<void>
{
  for (auto msg = co_await ctx.async_read(); msg;
       msg = co_await ctx.async_read()) {
    if (auto binary = std::get_if<websocket::binary_t>(&*msg); binary) {
      co_await ctx.async_write_binary(
          std::span { binary->value.data(), binary->value.size() });
    } else if (auto text = std::get_if<websocket::text_t>(&*msg); text) {
      co_await ctx.async_write_text(text->value);
    } else if (auto c = std::get_if<websocket::close_t>(&*msg); c) {
      break;
    }
  }
}

int main()
{
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/ws">([](websocket ws) -> awaitable<response> {
              co_return ws.set_handler(echo);
            }))
            .build();
  server.bind("127.0.0.1", 8080);

  ioc.run();
}
