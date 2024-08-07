//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

auto get_order_tuple(path_of<std::tuple<std::string, std::uint64_t>> order)
    -> awaitable<response>
{
  auto [user, order_id] = order;

  co_return response::ok()
      .set_header(http::field::content_type, mime::text_plain())
      .set_body(fmt::format("user: {}, order_id: {}", user, order_id));
}

struct order_t {
  std::string user;
  std::uint64_t order_id;
};

auto get_order_struct(path_of<order_t> order) -> awaitable<response>
{
  co_return response::ok()
      .set_header(http::field::content_type, mime::text_plain())
      .set_body(
          fmt::format("user: {}, order_id: {}", order.user, order.order_id));
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/tuple/users/{user}/orders/{order_id}">(
                        get_order_tuple))
                    .serve(route::get<"/struct/users/{user}/orders/{order_id}">(
                        get_order_struct))
                    .build();

  server.bind("127.0.0.1", 8080);

  ioc.run();
}
