//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

int main()
{
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/">(
                [](std::string body) -> awaitable<http_response> {
                  co_return http_response::ok()
                      .set_field(http::field::content_type,
                                 http::fields::content_type::plaintext())
                      .set_body(body);
                }))
            .build();
  server.bind("127.0.0.1", 8080);

  const std::size_t threads = std::thread::hardware_concurrency();
  auto tp = net::static_thread_pool(threads);
  for (std::size_t i = 0; i < threads; ++i) {
    net::post(ioc, [&]() { ioc.run(); });
  }
  ioc.run();
}
