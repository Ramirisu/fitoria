//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

#include <atomic>

using namespace fitoria;
using namespace fitoria::web;

using counter_t = std::atomic<std::size_t>;

auto index(state_of<std::shared_ptr<counter_t>> counter) -> awaitable<response>
{
  co_return response::ok()
      .set_header(http::field::content_type,
                  http::fields::content_type::plaintext())
      .set_body(fmt::format("index page has been called {} times.",
                            counter->fetch_add(1, std::memory_order_relaxed)));
}

int main()
{
  auto counter = std::make_shared<counter_t>(1);

  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">(index).use_state(counter))
                    .build();

  server.bind("127.0.0.1", 8080);

  ioc.run();
}
