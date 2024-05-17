//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

struct order_t {
  std::string user;
  std::uint64_t order_id;
};

auto get_order(query_of<order_t> order) -> awaitable<response>
{
  co_return response::ok()
      .set_header(http::field::content_type,
                  http::fields::content_type::plaintext())
      .set_body(
          fmt::format("user: {}, order_id: {}", order.user, order.order_id));
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/order">(get_order))
                    .build();

  server.bind("127.0.0.1", 8080);

  ioc.run();
}
